import os
import json

def getInputList(workingDir, iid, folderName, cores):
  # iid stands for input id
  jData = []
  for i in range(cores):
    crashDir = os.path.join(workingDir, 'afl-fuzz', 'out', str(i), folderName)
    for fileName in os.listdir(crashDir):
      filePath = os.path.join(crashDir, fileName)
      if fileName != 'README.txt' and os.path.isfile(filePath):
        data = {}
        data['iid'] = iid
        iid += 1
        data['file'] = filePath
        jData.append(data)
  return jData

def getInputPaths(workingDir, cores):
  jData = getInputList(workingDir, 1, 'crashes', cores)

  crashPath = os.path.join(workingDir, 'tmp', 'crash_inputs.json')
  with open(crashPath, 'w+') as f:
    json.dump(jData, f, indent=2)
  print('[INFO] - extracted %d crash inputs and saved on %s' % (len(jData), crashPath))

  jData = getInputList(workingDir, len(jData) + 1, 'queue', cores)
  nonCrashPath = os.path.join(workingDir, 'tmp', 'non_crash_inputs.json')
  with open(nonCrashPath, 'w+') as f:
    json.dump(jData, f, indent=2)
  print('[INFO] - extracted %d non-crash inputs and save on %s' % (len(jData), nonCrashPath))

