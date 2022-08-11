import os
import numpy as np
import matplotlib.pyplot as plt

def getScore(path):
    with open(path) as f:
        line = f.readlines()
        if len(line) < 3:
            return -1, -1, -1, -1
        N = int(line[0].lstrip("N = "))
        K = int(line[1].lstrip("K = "))
        density = float(line[2].lstrip("Density = "))

        score = line[-1]
        if 'Score = ' not in score:
            return -1, -1, -1, -1
        score = int(score.lstrip("Score = "))
        score_ratio = score / (K * 50 * 99)

    return score, N, K, density, score_ratio

def getCerr(path):
    with open(path) as f:
        line = f.readlines()
        if len(line) == 0:
            return -1
        time_ms = int(line[-1].lstrip("Time = "))
    return time_ms

if __name__ == '__main__':
    dir = 'score'
    cerr_dir = 'cerr'
    files = os.listdir(dir)
    sum, cnt = 0, 0
    max_score_sum = 0
    result = []
    for i, file in enumerate(files):
        score, n, k, density, score_ratio = getScore(os.path.join(dir, file))
        time_ms = getCerr(os.path.join(cerr_dir, file))
        if score < 0: 
            continue
        result.append(np.array([score, k, density, score_ratio]))
        sum += int(score)
        max_score_sum += k * 50 * 99
        # print(f"{file}: {score}")
        cnt += 1

        print(f'{file}, Score = {score}, N = {n}, K = {k}, Density = {round(density,3)}, Time = {time_ms}')
    result = np.array(result)

    # print("sum:", sum)
    print("Mean:", sum / cnt)
    print("Cnt:", cnt)
    print("Ratio:", sum / max_score_sum)
    print("Estimated 50 case score:", '{:_}'.format(sum * 50 / cnt))
    print("Estimated 50 case MAXIMUM score:", '{:_}'.format(max_score_sum * 50 / cnt))
    corr = np.corrcoef(result[:, 2], result[:, 0])[0, 1]
    print('Correlation of Score & Server Density', corr)
    # print('Max Score :', '{:_}'.format(10**8))
    # print("density:", (sum / cnt) / 10**6)

    fig = plt.figure(figsize = (15, 6), facecolor='lightblue')
    ax1 = fig.add_subplot(1, 3, 1)
    ax2 = fig.add_subplot(1, 3, 2)
    ax3 = fig.add_subplot(1, 3, 3)

    ax1.scatter(result[:, 1], result[:, 0])
    ax1.set_xlabel('K')
    ax1.set_ylabel('Score')

    ax2.scatter(result[:, 1], result[:, 3])
    ax2.set_xlabel('K')
    ax2.set_ylabel('Score (ratio)')

    ax3.scatter(result[:, 2], result[:, 0])
    ax3.set_xlabel('Server Density')
    ax3.set_ylabel('Score')

    plt.savefig('fig.png')