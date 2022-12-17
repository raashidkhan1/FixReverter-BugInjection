import pickle

if __name__ == '__main__':
  with open('crash_seeds.pickle', 'rb') as f:
    crash_seeds_by_trial = pickle.load(f)

  new_crash_seeds_by_trial = {}
  for trial_name, seeds in crash_seeds_by_trial.items():
    keep_seeds = []
    for seed in seeds:
      if 'run_combined' in seed and seed['run_combined']:
        keep_seeds.append(seed)
    new_crash_seeds_by_trial[trial_name] = keep_seeds

    with open('crash_seeds_new.pickle', 'wb+') as f:
      pickle.dump(new_crash_seeds_by_trial, f)
