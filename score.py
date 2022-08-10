import os

def getScore(path):
    with open(path) as f:
        line = f.readlines()
        if len(line) == 0:
            return -1
        score = line[-1]
        if 'Score = ' not in score:
            return -1
        score = int(score.lstrip("Score = "))
    return score

if __name__ == '__main__':
    dir = 'score'
    files = os.listdir(dir)
    sum = 0
    cnt = 0
    for i, file in enumerate(files):
        path = os.path.join(dir, file)
        score = getScore(path)
        if score < 0: 
            continue
        sum += int(score)
        # print(f"{file}: {score}")
        cnt += 1

    print("sum:", sum)
    print("mean:", sum / cnt)
    print("cnt:", cnt)
    print("estimated 50 case score:", '{:_}'.format(sum * 50 / cnt))
    # print('Max Score :', '{:_}'.format(10**8))
    # print("ratio:", (sum / cnt) / 10**6)