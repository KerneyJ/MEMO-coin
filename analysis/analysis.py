#!analysisenv/bin/python3
import matplotlib
import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns

from matplotlib.scale import LogScale
from matplotlib.lines import Line2D
from matplotlib.ticker import FuncFormatter, ScalarFormatter

sns.set(font_scale=5)
sns.set_theme(style="whitegrid")
figsize=(16,6.4)

def blockchain_throughput():
    f, ax = plt.subplots(nrows=1, ncols=1, figsize=figsize)
    clients = [1, 2, 4, 8]
    thrghpt = [0, 0, 1219.047619047619, 0]
    df = pd.DataFrame({"Clients": clients, "Throughput(transactions/second)": thrghpt})
    sns.lineplot(data=df, x="Clients", y="Throughput(transactions/second)", ax=ax)
    f.savefig("throughput.png");


    # ax.set_xlabel("Nodes", fontsize=16)
    # ax.set_ylabel("Average Time(s)", fontsize=16)
    # ax.set_title("Average Time to Import Tensorflow", fontsize=24)
    # f.savefig("importtf_nodes.png", transparent=True)

blockchain_throughput()
