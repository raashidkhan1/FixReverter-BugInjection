import os
import shutil
import multiprocessing

def copyWorkers(workingDir, workerDir, workers):
  if os.path.exists(workerDir):
    input('[WARNING] - %s already exists, press enter to remove or ctrl+c to exit' % workerDir)
    shutil.rmtree(workerDir)

  pool= multiprocessing.Pool(workers)
  inPuts = [(workingDir, workerDir, str(i)) for i in range(workers)]
  res = pool.map(copyWorker, inPuts)
  pool.close()
  pool.join()

def copyWorker(inPut):
  workingDir = inPut[0]
  workerDir = inPut[1]
  index = inPut[2]
  sourceDir = os.path.join(workingDir, 'src_injected')
  destDir = os.path.join(workerDir, index)
  shutil.copytree(sourceDir, destDir)
