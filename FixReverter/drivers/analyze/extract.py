import os
import sys
from inspect import getsourcefile



def splitInputs(tmpDir: str) -> list:
  with open(os.path.join(tmpDir, "crash_inputs.json"), "r") as f:
    inputs = json.load(f)
  with open(os.path.join(tmpDir, "non_crash_inputs.json"), "r") as f:
    inputs.extend(json.load(f))

  assignment = []
  for i in range(workers):
    assignment.append((i, inputs[i::workers]))

  return assignment

def getReachTrigger(workingDir: str, workers: int):
  current_path = os.path.abspath(getsourcefile(lambda:0))
  current_dir = os.path.dirname(current_path)
  parent_dir = current_dir[:current_dir.rfind(os.path.sep)]
  sys.path.insert(0, os.path.join(parent_dir, 'utils'))
  from runtarget import runTarget, extractStat

  tmpDir = os.path.join()

  assignment = splitInputs(tmpDir)
  pool= multiprocessing.Pool(workers)

  res = pool.map(work, assignment)
  pool.close()
  pool.join()

  inputs = []
  for r in res:
    inputs.extend(r)

  with open(os.path.join(tmpDir, "reachTrigger.json"), "w+") as f:
    json.dump(inputs, f, indent=2)

def work(assignment: tuple) -> list:
  worker = assignment[0]
  inputs = assignment[1]

  res = []
  for inPut in inputs:
    print('worker %s is working on iid %d' % (worker, inPut["iid"]))
    inPut["worker"] = worker
    targetPath = os.path.join(workerDir, str(worker))
    inPut = runTarget(worker, inPut)
    res.append(inPut)
  return res
