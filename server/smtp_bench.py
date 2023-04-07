#!/usr/bin/python3
import argparse
from joblib import Parallel, delayed
import re
import random
import smtplib
import time
import threading
from typing import List


def parse_size(size_str: str, regex=re.compile("^(\\d+)([kmg]?)$")) -> int:
    multipliers = {
        "k": 1000,
        "m": 1000000,
        "g": 1000000000,
    }
    size_str = size_str.lower()
    match = regex.match(size_str)
    if not match:
        raise ValueError()
    size = int(match[1])
    if match[2]:
        size *= multipliers[match[2]]
    return size


def generate_mail_text(size: int) -> str:
    return "".join(random.choices("abcdefghijklmnopqrstuvwxyz", k=size))


def generate_mail_recipients(recipients: int) -> List[str]:
    return [f"recipient{i}@test" for i in range(recipients)]


def run_connection(thread_id: int, host: str, port: int, mail_size: int, mail_recipients: int, count: int):
    mail = generate_mail_text(mail_size)
    recipients = generate_mail_recipients(mail_recipients)
    sender = f"sender{thread_id}@test"
    latency = 0
    with smtplib.SMTP(host=host, port=port) as connection:
        for i in range(count):
            start = time.perf_counter_ns()
            connection.sendmail(sender, recipients, mail)
            latency += time.perf_counter_ns() - start
    return latency * 1.0 / count


def main():
    parser = argparse.ArgumentParser(prog="smtp_bench")
    parser.add_argument("--host", required=True)
    parser.add_argument("--port", "-p", default=465, type=int)
    parser.add_argument("--mail-size", default="1K")
    parser.add_argument("--mail-recipients", default=1, type=int)
    parser.add_argument("--connections", default=1, type=int)
    parser.add_argument("--count", default="1K")
    args = parser.parse_args()

    host = args.host
    port = args.port
    mail_size = parse_size(args.mail_size)
    mail_recipients = args.mail_recipients
    connections = args.connections
    count = parse_size(args.count)
    latencies = Parallel(n_jobs=connections)([delayed(run_connection)(i, host, port, mail_size, mail_recipients, count) for i in range(connections)])
    print(sum(latencies) / len(latencies) / 1e6, "ms")


if __name__ == "__main__":
    main()
