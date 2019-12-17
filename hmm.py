import numpy as np
from hmmlearn import hmm
import pickle
import os

iterations = 10

with open('accel_arr', 'rb') as file:
     accel_arr= pickle.load(file)

states = ["balance", "losing_balance", "impact"]
n_states = len(states)
observations = [0, 1, 2, 3, 4, 5, 6, 7]
n_observations = len(observations)

model = hmm.MultinomialHMM(n_components=n_states, n_iter=20, tol=0.01)
x = np.array([])

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

print("startprob =", )
with open('pi_A_B','wb') as f:
    pickle.dump(pi_A_B, f)