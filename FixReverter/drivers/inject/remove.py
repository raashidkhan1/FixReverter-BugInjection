import os
import sys
import itertools
from inspect import getsourcefile

currentFile = os.path.abspath(getsourcefile(lambda:0))
ParentDir = os.path.dirname(os.path.dirname(currentFile))
sys.path.insert(0, os.path.join(ParentDir, 'utils'))
from runtarget import runTarget, extractStat
  

def removeSeedTriggers(seedsDir: str, exePath:str, runOptions='-exact_artifact_path=/dev/null') -> list:
  if not os.path.exists(exePath):
    print('[ERROR] - executable not found: %s' % exePath)
    sys.exit(0)

  inputPaths = os.listdir(seedsDir)

  muteIndices = set()
  myEnv = os.environ.copy()

  for i in inputPaths:
    retcode = -1
    inputPath = os.path.join(seedsDir, i)

    while retcode != 0:
      envStr = 'off ' + ' '.join([str(i) for i in muteIndices])
      myEnv['FIXREVERTER'] = envStr
      print(f'[INFO] - running testcase with env variable FIXREVERTER {envStr}')
      retcode, triggers = runWithOff(exePath, inputPath, myEnv, runOptions)
      if retcode == 0:
        print('[INFO] - no crash, proceed to next testcase')
        break
      elif len(triggers) == 0:
        print(f'[ERROR] - seed crashes with no triggering at all: inputPath')
        sys.exit(0)

      print('[INFO] - crashes with trigger %s, extract minimal crash set' % str(triggers))
      crashsets = []
      # get minimal crash sets
      for n in range(1, len(triggers)+1):
        for currset in itertools.combinations(triggers, n):
          # skip if current set is superset of a crash set
          isSuper = False
          for crashset in crashsets:
            if set(currset).issuperset(crashset):
              isSuper = True
              break
          if isSuper:
            continue
            
          myEnv["FIXREVERTER"] = 'on ' + ' '.join([str(i) for i in currset])
          curr_retcode, _ = runWithOff(exePath, inputPath, myEnv, runOptions)
          if curr_retcode != 0:    # found a minimal crash set
            crashsets.append(currset)
            muteIndices.update(currset)
            print('[INFO] - extend muteIndices with %s' % str(currset))

  print('[INFO] - %d injections are crashing seed(s)' % len(muteIndices))
  print('[INFO] - they are %s' % str(sorted(muteIndices)))
  return sorted(muteIndices)

def runWithOff(exePath, inputPath, myEnv, runOptions):
  print('[INFO] - running seed %s with env variable FIXREVERTER with value of "%s"' % (inputPath, myEnv['FIXREVERTER']))
  retcode, err = runTarget(exePath, inputPath, myEnv, runOptions)
  inPut = {'path' : inputPath}
  inPut = extractStat(inPut, err, 2)    #we are only interested in trigger here
  print('[INFO] - triggered injection %s' % str(inPut['trigger']))
  return (retcode, inPut['trigger'])
