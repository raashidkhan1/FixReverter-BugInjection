import random
import itertools

def chooseCondExe(jData):
  random.seed(1)

  for j in jData:
    if j['pattern'] == 'COND_EXEC':
      trace_vars = []
      for i in range(j['ids']):
        var = j['ids'][i]
        if 'traceVar' in var['tag']:
          trace_vars.append(i)

      if len(trace_vars) > 1:
        choice = random.choice(trace_vars)
        trace_vars.remove(choice)
        for i in trace_vars:
          j['id'][i].pop('crash', None)

  edges = []
  size = len(jData)
  for pair in itertools.permutations(range(len(jData)), 2):
    first = jData[pair[0]]
    second = jData[pair[1]]

    if first['fileName'] == second['fileName']:
      # first is in range of second
      if (int(first['stmt']['start']['line']) > int(second['stmt']['start']['line'])
          or (int(first['stmt']['start']['line']) == int(second['stmt']['start']['line'])
            and int(first['stmt']['start']['col']) > int(second['stmt']['start']['col'])) 
        and (int(first['stmt']['end']['line']) < int(second['stmt']['end']['line']
          or (int(first['stmt']['end']['line']) == int(second['stmt']['end']['line'])
            and int(first['stmt']['end']['col']) < int(second['stmt']['end']['col'])):
        edges.append(pair)

  nodes = set()
  outers = set()    
  for edge in edges:
    nodes.add(edge[0])
    nodes.add(edge[1])
    outers.add(edge[1])

  inners = [for n in nodes if n not in outers]

  



  return jData



