import shutil
import os
import json
from preinject import copySrcCode

def cleanDir(dirPath: str):
  print('[INFO] - removing tmp folder %s' % dirPath)
  if os.path.exists(dirPath):
    shutil.rmtree(dirPath)

def renameDir(dirPath: str, name: str):
  parentDir = os.path.dirname(dirPath)
  os.rename(dirPath, os.path.join(parentDir, name))


def clean(workingDir: str, srcDir: str, tmpDir: str, entryFile: str, rmIndices: str):
  cleanDir(os.path.join(tmpDir, 'injectTmp'))
  cleanDir(os.path.join(tmpDir, 'mainTmp'))

  bkDir = os.path.join(tmpDir, 'backup')
  copySrcCode(srcDir, os.path.join(bkDir, 'prog_before_remove'))
  shutil.rmtree(srcDir)
  shutil.copytree(os.path.join(bkDir, 'prog_before_inject'), srcDir)

  shutil.move(entryFile, os.path.join(bkDir, 'entry_before_remove'))
  shutil.copyfile(os.path.join(bkDir, 'entry_before_inject'), entryFile)
