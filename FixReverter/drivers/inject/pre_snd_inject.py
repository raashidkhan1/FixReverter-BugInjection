import json
import os
from preinject import getFilesDict

def removeAPM(apmPath: str, rmIndices: list) -> list:
  with open(apmPath) as f:
    jData = json.load(f)

  jData = [j for j in jData if j['index'] not in rmIndices]
  return jData


def pre_snd_inject(tmpDir: str, rmIndices: str):
  apmPath = os.path.join(tmpDir, 'inject.json')
  jData = removeAPM(apmPath, rmIndices)
  with open(apmPath, 'w+') as f:
    json.dump(jData, f, indent=2)

  filesDict = getFilesDict(jData)

  return jData[-1]['index'], filesDict
