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
  parser.add_argument('-a', '--all', action='store_true', help='run all tasks in the tool chain')
  parser.add_argument('-p', '--pattern', action='store_true', help='run ast pattern matcher on all c files')
  parser.add_argument('-i', '--inject', action='store_true', help='inject matched patterns into program')
  parser.add_argument('-r', '--remove', action='store_true', help='remove boring injections that are triggered by fuzzing seed(s) and reinject the remaining injections')
  args = parser.parse_args()

    #set up config
  if args.config:
    if validConfigPath(args.config):
      configPath = args.config
    else:
      sys.exit(0)
  else:
    defConfig = 'FixReverter.config'
    print('[INFO] - config option not specified, trying file: %s' % defConfig)
    if validConfigPath(defConfig):
      configPath = defConfig
    else:
      sys.exit(0)

  config = configparser.RawConfigParser(allow_no_value=True)
  config.optionxform = str
  config.read(configPath)
  #make sure all paths exist
  for k,v in config.items('paths'):
    if not os.path.exists(v):
      print('[ERROR] - "%s" option in "paths" section does not exist: %s' % (k, v))
      sys.exit(0)

  #extract values from config
  try:
    workingDir = config.get('paths', 'working dir')
    srcDir = config.get('paths', 'program source')
    compileDBPath = config.get('paths', 'compilation database')
    matcherPath = config.get('paths', 'pattern matcher')
    grammarPath = config.get('paths', 'ast grammer')
    mainLocatorPath = config.get('paths', 'main function locator')
    mutantLocatorPath = config.get('paths', 'mutant locator')
    rewriterPath = config.get('paths', 'rewriter')
    entryFile = os.path.realpath(config.get('paths', 'entry file'))
    setFlagCodePath = config.get('paths', 'setflag codes')
    seedsDir = config.get('paths', 'seeds')
    entryFunc = config.get('values', 'entry function')
    cores = int(config.get('values', 'cores'))
    fuzzerInclusions = config.get("values", "fuzzer inclusions") if config.has_option("values", "fuzzer inclusions") else None
    exePath = config.get('optional paths', 'exe')
  except configparser.NoSectionError as e:
    print('[ERROR] - missing config section: %s' % str(e))
    sys.exit(0)
  except configparser.NoOptionError as e:
    print('[ERROR] - missing config value: %s' % str(e))
    sys.exit(0)
  if config.has_section('build savers'):
    buildSaves = [os.path.realpath(p) for p in config.options('build savers')]
    for bs in buildSaves:
      print(f'[INFO] - found file in build savers option {bs}')
  else:
    buildSaves = []
  if config.has_section('ignores'):
    ignores = [os.path.realpath(p) for p in config.options('ignores')]
  else:
    ignores = [entryFile]

  #create tmp dir
  tmpDir = os.path.join(workingDir, "tmp")
  if not os.path.exists(tmpDir):
    os.mkdir(tmpDir)

  #handle jobs based on command line flags
  if args.pattern or args.all:
    print('[INFO] - running AST pattern matcher')
    import runapm
    runapm.runapm(tmpDir, srcDir, compileDBPath, matcherPath, grammarPath)

  if args.inject or args.all:
    print('[INFO] - injecting AST patterns into source code')
    import inject
    import preinject
    indices, filesDict = preinject.preinject(srcDir, workingDir, entryFile,
                                             ignores)
    inject.inject(srcDir, workingDir,
                    mainLocatorPath,
                    mutantLocatorPath, rewriterPath,
                    entryFile, entryFunc, 
                    compileDBPath, setFlagCodePath,
                    fuzzerInclusions, buildSaves,
                    cores, indices, filesDict)

  if args.remove or args.all:
    print('[INFO] - removing boring injections')
    import remove
    rmIndices = remove.removeSeedTriggers(seedsDir, exePath)
    import clean
    clean.clean(workingDir, srcDir, tmpDir, entryFile, rmIndices)
    import pre_snd_inject
    sndIndices, sndFilesDict = pre_snd_inject.pre_snd_inject(tmpDir, rmIndices)


    print('[INFO] - injecting AST patterns into source code second time')
    import inject
    inject.inject(srcDir, workingDir,
                    mainLocatorPath,
                    mutantLocatorPath, rewriterPath,
                    entryFile, entryFunc, 
                    compileDBPath, setFlagCodePath,
                    fuzzerInclusions, buildSaves,
                    cores, sndIndices, sndFilesDict)

    
