import os
import sys
import json
import configparser
import argparse

def validConfigPath(path):
  if os.path.exists(path):
    if os.path.isfile(path):
      return True
    else:
      print('[ERROR] - config path is a folder: %s' % path)
  else:
    print('[ERROR] - config path does not exist: %s' % path)
  return False

if __name__ == '__main__':
  #parse command line options
  parser = argparse.ArgumentParser()
  parser.add_argument('--config', help='path to config file, will try to search on current directory if left empty')
  parser.add_argument('-w', '--worker', action='store_true', help='copy number of workers specified in config from compare folder')
  parser.add_argument('-m', '--min', action='store_true', help='calculate min crash set for crashing inputs')
  parser.add_argument('-i', '--inputs', action='store_true', help='extract input paths from AFL results')
  parser.add_argument('-e', '--extract', action='store_true', help='extract reach and trigger for each input')
  parser.add_argument('-c', '--count', action='store_true', help='count reach and trigger numbers')
  parser.add_argument('-s', '--split', choices={'apm', 'sum', 'both'}, help='split matched patterns by pattern type')
  args = parser.parse_args()
  
  #set up config
  if args.config:
    if validConfigPath(args.config):
      configPath = args.config
    else:
      sys.exit(0)
  else:
    defConfig = 'tool_chain_config'
    print('[INFO] - config option not specified, trying file: %s' % defConfig)
    if validConfigPath(defConfig):
      configPath = defConfig
    else:
      sys.exit(0)      
      
  config = configparser.RawConfigParser()
  config.read(configPath)
  #extract values from config
  try:
    workingDir = config.get('paths', 'working dir')
    relExe = config.get('paths', 'relative exe')
    fuzzFlags = config.get('values', 'fuzz flags')
    cores = int(config.get('values', 'cores'))
    workers = int(config.get('values', 'workers'))
  except configparser.NoSectionError as e:
    print('[ERROR] - missing config section: %s' % str(e))
    sys.exit(0)
  except configparser.NoOptionError as e:
    print('[ERROR] - missing config value: %s' % str(e))
    sys.exit(0)

  workerDir = os.path.join(workingDir, 'workers')
  
  #handle jobs based on command line flags
  if args.worker:
    print('[INFO] - copying workers from compare folder')
    import copyworkers
    workerDir = os.path.join(workingDir, 'workers')
    copyworkers.copyWorkers(workingDir, workerDir, workers)

  if args.min:
    raise NotImplementedError('not migrated to this implementation yet!')

  if args.inputs:
    print('[INFO] - extracting fuzzing input paths')
    import getinputs
    getinputs.getInputPaths(workingDir, cores)

  if args.extract:
    print('[INFO] - calculating reach and trigger for each input')
    import getinputs
    getinputs.getInputPaths(workingDir, cores)

  if args.count:
    print('[INFO] - counting reach and trigger numbers')
    import count
    sumDict = count.countReachTrigger(workingDir)

  if args.split:
    if args.split == 'apm':
      splitApm = True
    if args.split == 'sum':
      splitSum = True
    if args.split == 'both':
      splitApm = True
      splitSum = True

    with open(os.path.join(workingDir, 'tmp', 'apm.json'), 'r') as f:
      apmData = json.load(f)

    import splitpattern
    if splitApm:
      print('[INFO] - counting pattern types on apm.json')
      patternDict = splitpattern.splitPattern(apmData)
      print(patternDict)

    if splitSum:
      #get summary dict if not already calculated with option --count
      if not args.count:
        import count
        sumDict = count.countReachTrigger(workingDir, printNum=False)
      #split pattern types in reach
      print('[INFO] - counting pattern types on reached indices')
      reachList = sumDict["reach"]
      reachData = [j for j in apmData if j["index"] in reachList]
      patternDict = splitpattern.splitPattern(reachData)
      print(patternDict)

      print('[INFO] - counting pattern types on triggered indices')
      triggerList = sumDict["trigger"]
      triggerData = [j for j in apmData if j["index"] in triggerList]
      patternDict = splitpattern.splitPattern(triggerData)
      print(patternDict)
    
    
  
    
    
