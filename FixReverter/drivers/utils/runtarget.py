import subprocess
import shlex

def runTarget(targetPath: str, inputPath: str, myEnv: dict, runOptions: str) -> list:
  fromStdIn = '<' in runOptions
  args =  shlex.split(" ".join([targetPath, runOptions, inputPath]))

  if fromStdIn:
    with open(inputPath, "rb") as f:
      p = subprocess.Popen(args, stdin=f, stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=myEnv)
  else:
    p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=myEnv)
  try:
    out, err = p.communicate(timeout=1)
  except subprocess.TimeoutExpired as e:
    p.kill()
    out, err = p.communicate()

  retcode = p.returncode
  return (retcode, err)

def extractStat(inPut: dict, err: list, mask: int) -> dict:
  if not mask:
    return inPut

  getReach = 1 & mask
  getTrigger = 2 & mask
  getError = 4 & mask
  
  if getReach:
    inPut["reach"] = set()
  if getTrigger:
    inPut["trigger"] = set()
  if err:
    lines = err.split(b'\n')
    for line in lines:
      if b'triggered bug index' in line:
        bugId = int(line.split(b' ')[-1].decode("utf-8"))
        if getTrigger:
          inPut["trigger"].add(bugId)
        if getReach:
          inPut["reach"].add(bugId)
      elif b'reached bug index' in line:
        bugId = int(line.split(b' ')[-1].decode("utf-8"))
        if getReach:
          inPut["reach"].add(bugId)
      elif b'ERROR: AddressSanitizer' in line:
        if getError:
          inPut["error"] = line.decode("utf-8")
    
    if getReach:
      inPut["reach"] = list(inPut["reach"])
    if getTrigger:
      inPut["trigger"] = list(inPut["trigger"])
  return inPut
