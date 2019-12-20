import numpy as np
from hmmlearn import hmm
import csv
import os

iterations = 10

x = []
for i in range(50):
    tmp = []
    with open('./fall_data/ads_acc_' + str(i) + '.csv', newline='') as csvfile:
        rows = csv.reader(csvfile)
        for row in rows:
            tmp.append(row)
        x.append(tmp)

for i in range(len(x)):
    for j in range(len(x[0])):
        if x[i][j] > 11:
            x[i][j] = 7
        elif x[i][j] > 8.7:
            x[i][j] = 6
        elif x[i][j] > 6.6:
            x[i][j] = 5
        elif x[i][j] > 4.7:
            x[i][j] = 4
        elif x[i][j] > 3.4:
            x[i][j] = 3
        elif x[i][j] > 2.2:
            x[i][j] = 2
        elif x[i][j] > 1.1:
            x[i][j] = 1
        else:
            x[i][j] = 0

x = np.asarray(x)



states = ["balance", "losing_balance", "impact"]
n_states = len(states)
observations = [0, 1, 2, 3, 4, 5, 6, 7]
n_observations = len(observations)

model = hmm.MultinomialHMM(n_components=n_states, n_iter=20, tol=0.01)

best_score = 0
startprob = np.zeros(len(states))
transmat = np.zeros((len(states), len(states)))
emissionprob = np.zeros((len(states), len(observations)))


for i in range(iterations):
    model.fit(x)
    if model.score(x) > best_score:
        startprob = model.startprob_
        transmat = model.transmat_
        emissionprob = model.emissionprob_
        best_score = model.score(x)

pi_A_B = []
pi_A_B.append(startprob.tolist())
pi_A_B.append(transmat.tolist())
pi_A_B.append(emissionprob.tolist())

print("startprob = ", pi_A_B[0])
print("transmat = ", pi_A_B[1])
print("emissionprob = ",pi_A_B[2])
with open('pi_A_B','wb') as f:
    pickle.dump(pi_A_B, f)