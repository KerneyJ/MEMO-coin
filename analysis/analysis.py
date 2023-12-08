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
    thrghpt01 = [435.3741496598639, 735.6321839080459, 1219.047619047619, 1306.1224489795918]
    thrghpt12 = [428.09364548494983, 820.5128205128206, 1132.7433628318583, 805.0314465408806]
    df = pd.DataFrame({"Clients": clients, "Throughput(transactions/second) 1 Validator": thrghpt01, "Throughput(transactions/second) 12 Validators": thrghpt12})
    sns.lineplot(data=df, x="Clients", y="Throughput(transactions/second) 1 Validator", ax=ax)
    sns.lineplot(data=df, x="Clients", y="Throughput(transactions/second) 12 Validators", ax=ax)
    ax.set_title("Throughput vs Clients with 1 validator")
    ax.set_ylabel("Throughput(tps)", fontsize=16)
    f.savefig("throughput.png");

    # ax.set_xlabel("Nodes", fontsize=16)
    # ax.set_ylabel("Average Time(s)", fontsize=16)
    # ax.set_title("Average Time to Import Tensorflow", fontsize=24)
    # f.savefig("importtf_nodes.png", transparent=True)

blockchain_throughput()
