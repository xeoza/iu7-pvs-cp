import argparse
import pandas as pd
from matplotlib import pyplot


def memory_plot(location, data):
    malloc = data[["begin_x", "size_x"]]
    malloc["time"] = malloc["begin_x"].rename("time") / 1e9
    malloc["size"] = malloc["size_x"].rename("size")
    free = data[["end_y", "size_x"]]
    free["time"] = free["end_y"].rename("time") / 1e9
    free["size"] = -free["size_x"]
    data = pd.concat([malloc[["time", "size"]], free[["time", "size"]]])
    data = data.sort_values(by="time")
    data["size"] = data["size"].cumsum().astype(int)
    data.plot(title=location, legend=False, x="time", y="size", xlabel="Время, с", ylabel="Размер выделенной памяти, байт")


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("profile")
    parser.add_argument("--visual", action="store_true")
    parser.add_argument("--count-threshold", default=10, type=int)
    parser.add_argument("--size-threshold", default=100, type=int)
    return parser.parse_args()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("profile")
    parser.add_argument("--visual", action="store_true")
    parser.add_argument("--count-threshold", default=10, type=int)
    parser.add_argument("--size-threshold", default=100, type=int)
    args = parse_args()

    profile = pd.read_csv(args.profile, sep="\t", names=["type", "address", "size", "amount", "location", "begin", "end"])
    joined = pd.merge_asof(profile[profile["type"] == "MALLOC"], profile[profile["type"] == "FREE"], by="address", left_on="end", right_on="begin", direction="forward")
    joined = joined.dropna()
    joined["begin_y"] = joined["begin_y"].astype(pd.Int64Dtype())
    joined["end_y"] = joined["end_y"].astype(pd.Int64Dtype())
    for location, data in joined.groupby("location_x"):
        if args.visual:
            memory_plot(location, data)
        if data["size_x"].count() < args.count_threshold:
            continue
        if data["size_x"].max() < args.size_threshold:
            continue
        if (data["size_x"] == data["size_x"].iloc[0]).all():
            size = data["size_x"].iloc[0]
            print(location, "POOL", size, data["size_x"].sum() / size, sep="\t")
    if args.visual:
        pyplot.show()


if __name__ == "__main__":
    main()
