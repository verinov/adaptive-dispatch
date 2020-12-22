import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt

BM_OUTPUT = "bm_results.csv"
IMG_DIR = "benchmark_results"

data = pd.read_csv(BM_OUTPUT, skiprows=9)
data = data[~data.name.apply(lambda name: "_" in name.split("/")[-1])]
data = pd.DataFrame(
    {
        **{
            name: data.name.apply(func)
            for name, func in [
                ("case", lambda name: name.split("/")[0]),
                ("size", lambda name: int(name.split("/")[-1])),
                ("Implementation", lambda name: name.split("/")[1]),
            ]
        },
        "time": data.real_time,
    }
)

for case, case_df in data.groupby("case"):
    case_df = case_df.copy()
    min_times = case_df.groupby("size").aggregate("time").min()
    case_df["relative_time"] = (
        case_df["time"].values / min_times[case_df["size"]].values
    )

    sns.set(style="whitegrid")

    plt.figure(figsize=(16, 12))
    plt.title(case)
    sns.despine(bottom=True, left=True)

    sns.stripplot(
        y="relative_time",
        x="size",
        hue="Implementation",
        data=case_df,
        dodge=True,
        alpha=0.5,
        zorder=1,
    )
    plt.savefig(f"{IMG_DIR}/{case}.png")
