import os
import numpy as np
import matplotlib.pyplot as plt
from scipy.stats import gaussian_kde

def getScore(path):
    with open(path) as f:
        line = f.readlines()
        if len(line) < 3:
            return -1, -1, -1, -1, -1
        N = int(line[0].lstrip("N = "))
        K = int(line[1].lstrip("K = "))
        density = float(line[2].lstrip("Density = "))

        score = line[-1]
        if 'Score = ' not in score:
            return -1, -1, -1, -1, -1
        score = int(score.lstrip("Score = "))
        score_ratio = score / (K * 50 * 99)

    return score, N, K, density, score_ratio

def getCerr(path):
    with open(path) as f:
        line = f.readlines()
        if len(line) == 0:
            return -1
        solver = line[0]
        time_ms = int(line[-1].lstrip("Time = "))
        if 'Connect' not in line[-2]:
            connect_time_ms = -1
        else:
            connect_time_ms = int(line[-2].lstrip("Connect Time = "))
        is_dense = 1 if (solver=='Solver: Dense\n') else 0
    return time_ms, connect_time_ms, is_dense

def getOut(path):
    with open(path) as f:
        line = f.readlines()
        if len(line) == 0:
            return -1, -1
        M = int(line[0])
        C = int(line[M + 1])
    return M, C


if __name__ == '__main__':
    dir = 'score'
    cerr_dir = 'cerr'
    out_dir = 'out'
    files = os.listdir(dir)
    sum, cnt = 0, 0
    max_score_sum = 0
    result = []
    max_time = -1

    move_count = []
    for i, file in enumerate(files):
        score, n, k, density, score_ratio = getScore(os.path.join(dir, file))
        time_ms, connect_time_ms, is_dense = getCerr(os.path.join(cerr_dir, file))
        M, C = getOut(os.path.join(out_dir, file))
        max_time = max(max_time, time_ms)
        if score < 0: 
            continue
        result.append(np.array([score, k, density, score_ratio]))
        move_count.append(np.array([k, M, k*100 - M - C, score, density, time_ms, n, is_dense]))
        sum += int(score)
        max_score_sum += k * 50 * 99
        # print(f"{file}: {score}")
        cnt += 1

        # print(f'{file}, Score = {score}, N = {n}, K = {k}, Density = {round(density,3)}, Time = {time_ms}, Connect Time = {connect_time_ms}')
    
    result = np.array(result)
    move_count = np.array(move_count)
    
    print('-------------')
    if True:
        print(f'Dense, N={(move_count[:, 7] == 1).sum()}')
        fig = plt.figure(figsize = (16, 7), facecolor='lightblue')
        ax = [fig.add_subplot(1, 4, i+1) for i in range(4)]
        for i in range(2, 6) : 
            idx = np.logical_and(move_count[:, 0] == i, move_count[:, 7] == 1)
            mean = move_count[idx, 1].mean()
            rem = move_count[idx, 2].mean()
            rem_max = move_count[idx, 2].max()
            score = move_count[idx, 3].mean()
            ti = move_count[idx, 5].mean()
            n_time_corr = np.corrcoef(move_count[idx, 5], move_count[idx, 6])[0,1]
            time_max = move_count[idx, 5].max()
            print(f'K={i} (N={idx.sum()}) : MoveCount = {round(mean,1)}, 残り = {round(rem,1)}, Score = {round(score,1)}, MeanTime = {round(ti,1)}, MaxTime = {round(time_max,1)}, N-Time-Corr = {round(n_time_corr,3)}')
            print(f'残り MAX = {rem_max}')

            n = move_count[idx, 6]
            n = n - n.min()
            ti = move_count[idx, 5]
            xy = np.vstack([n, ti])
            z = gaussian_kde(xy)(xy)
            idx = z.argsort()
            x, y, z = n[idx], ti[idx], z[idx]
            ax[i-2].scatter(x, y, c=z)
            ax[i-2].set_xlabel('N')
            ax[i-2].set_ylabel('Time')
            ax[i-2].set_title(f'K = {i}')
        plt.savefig('D_fig_N_time.png')
        plt.close()
        
        fig = plt.figure(figsize = (16, 7), facecolor='lightblue')
        ax = [fig.add_subplot(1, 4, i+1) for i in range(4)]
        for i in range(2, 6) : 
            idx = np.logical_and(move_count[:, 0] == i, move_count[:, 7] == 1)
            n = move_count[idx, 6]
            n = n - n.min()
            mv = move_count[idx, 1]
            xy = np.vstack([n, mv])
            z = gaussian_kde(xy)(xy)
            idx = z.argsort()
            x, y, z = n[idx], mv[idx], z[idx]
            ax[i-2].scatter(x, y, c=z)
            ax[i-2].set_xlabel('N')
            ax[i-2].set_ylabel('Move Count')
            ax[i-2].set_title(f'K = {i}')
        plt.savefig('D_fig_N_Move.png')
        plt.close()
        print('-------------')

    if True:

        print(f'Sparse, N={(move_count[:, 7] == 0).sum()}')
        fig = plt.figure(figsize = (16, 7), facecolor='lightblue')
        ax = [fig.add_subplot(1, 4, i+1) for i in range(4)]
        for i in range(2, 6) : 
            idx = np.logical_and(move_count[:, 0] == i, move_count[:, 7] == 0)
            mean = move_count[idx, 1].mean()
            rem = move_count[idx, 2].mean()
            rem_max = move_count[idx, 2].max()
            score = move_count[idx, 3].mean()
            ti = move_count[idx, 5].mean()
            n_time_corr = np.corrcoef(move_count[idx, 5], move_count[idx, 6])[0,1]
            time_max = move_count[idx, 5].max()
            print(f'K={i} (N={idx.sum()}) : MoveCount = {round(mean,1)}, 残り = {round(rem,1)}, Score = {round(score,1)}, MeanTime = {round(ti,1)}, MaxTime = {round(time_max,1)}, N-Time-Corr = {round(n_time_corr,3)}')
            print(f'残り MAX = {rem_max}')

            n = move_count[idx, 6]
            n = n.max() - n
            ti = move_count[idx, 5]
            xy = np.vstack([n, ti])
            z = gaussian_kde(xy)(xy)
            idx = z.argsort()
            x, y, z = n[idx], ti[idx], z[idx]
            ax[i-2].scatter(x, y, c=z)
            ax[i-2].set_xlabel('N_margin')
            ax[i-2].set_ylabel('Time')
            ax[i-2].set_title(f'K = {i}')
        plt.savefig('S_fig_N_time.png')
        plt.close()

        fig = plt.figure(figsize = (16, 7), facecolor='lightblue')
        ax = [fig.add_subplot(1, 4, i+1) for i in range(4)]
        for i in range(2, 6) : 
            idx = np.logical_and(move_count[:, 0] == i, move_count[:, 7] == 0)
            n = move_count[idx, 6]
            n = n.max() - n
            mv = move_count[idx, 1]
            xy = np.vstack([n, mv])
            z = gaussian_kde(xy)(xy)
            idx = z.argsort()
            x, y, z = n[idx], mv[idx], z[idx]
            ax[i-2].scatter(x, y, c=z)
            ax[i-2].set_xlabel('N_margin')
            ax[i-2].set_ylabel('Move Count')
            ax[i-2].set_title(f'K = {i}')
        plt.savefig('S_fig_N_Move.png')
        plt.close()
        print('-------------')
    

    # print("sum:", sum)
    print("Mean:", sum / cnt)
    print("Cnt:", cnt)
    print("Ratio:", sum / max_score_sum)
    print("Estimated 50 case score:", '{:_}'.format(sum * 50 / cnt))
    print("Estimated 50 case MAXIMUM score:", '{:_}'.format(max_score_sum * 50 / cnt))
    corr = np.corrcoef(result[:, 2], result[:, 0])[0, 1]
    print('Correlation of Score & Server Density', corr)
    print(f'Max Time = {max_time}')
    # print('Max Score :', '{:_}'.format(10**8))
    # print("density:", (sum / cnt) / 10**6)

    fig = plt.figure(figsize = (11, 11), facecolor='lightblue')
    ax1 = fig.add_subplot(2, 2, 1)
    ax2 = fig.add_subplot(2, 2, 2)
    ax3 = fig.add_subplot(2, 2, 3)
    ax4 = fig.add_subplot(2, 2, 4)

    ax1.scatter(result[:, 1], result[:, 0])
    ax1.set_xlabel('K')
    ax1.set_ylabel('Score')

    ax2.scatter(result[:, 1], result[:, 3])
    ax2.set_xlabel('K')
    ax2.set_ylabel('Score (ratio)')

    ax3.scatter(result[:, 2], result[:, 0])
    ax3.set_xlabel('Server Density')
    ax3.set_ylabel('Score')

    ax4.scatter(result[:, 2], result[:, 3])
    ax4.set_xlabel('Server Density')
    ax4.set_ylabel('Score (ratio)')

    plt.savefig('fig.png')