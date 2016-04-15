#
# A minhash example
# Description: calculate the number of reachable nodes
#
import random, sys, pdb

graph = {}
reverse_graph = {}
results = {}

def parse_input():
    with open('Wiki-Vote.txt', 'r') as f:
#    for debug
#    with open('test.txt', 'r') as f:
#    with open('test1.txt', 'r') as f:
#    with open('test2.txt', 'r') as f:
        for line in f:
            if not line.startswith('#'):
                tmp_str = line.split()
                if int(tmp_str[0]) in graph:
                    if int(tmp_str[1]) not in graph[int(tmp_str[0])][0]:
                        graph[int(tmp_str[0])][0].append(int(tmp_str[1]))
                else:
                    graph[int(tmp_str[0])] = [[int(tmp_str[1])], 0, 0]

                if int(tmp_str[1]) in graph:
                    if int(tmp_str[1]) not in graph[int(tmp_str[1])][0]:
                        graph[int(tmp_str[1])][0].append(int(tmp_str[1]))
                else:
                    graph[int(tmp_str[1])] = [[int(tmp_str[1])], 0, 0]

                if int(tmp_str[0]) in reverse_graph:
                    if int(tmp_str[0]) not in reverse_graph[int(tmp_str[0])][0]:
                        reverse_graph[int(tmp_str[0])][0].append(int(tmp_str[0]))
                else:
                    reverse_graph[int(tmp_str[0])] = [[int(tmp_str[0])], 0, 0]

                if int(tmp_str[1]) in reverse_graph:
                    if int(tmp_str[0]) not in reverse_graph[int(tmp_str[1])][0]:
                        reverse_graph[int(tmp_str[1])][0].append(int(tmp_str[0]))
                else:
                    reverse_graph[int(tmp_str[1])] = [[int(tmp_str[0])], 0, 0]

                for k,v in reverse_graph.items():
                    results[k] = []
            else:
                pass

def assign_value():
    for k,v in reverse_graph.items():
        v[1] = random.random()

def find_minimum():
    minval = 2
    mink = 3
    for k,v in reverse_graph.items():
        if minval > v[1] and v[2] == 0:
            minval = v[1]
            mink = k

    return (minval, mink)

def update_minimum(begin_key, min_key, min_value):
    # do dfs
    for node in reverse_graph[min_key][0]:
        if node != begin_key and node != min_key and reverse_graph[node][2] == 0:
            update_minimum(begin_key, node, min_value)
        else:
            if reverse_graph[node][2] == 0:
                reverse_graph[node][1] = min_value
                reverse_graph[node][2] = 1

    if reverse_graph[min_key][2] == 0:
        reverse_graph[min_key][1] = min_value
        reverse_graph[min_key][2] = 1

def dfs_degree(origin, begin):
    stack = []
    stack.append(origin)
    pops = []
    begin = stack.pop()
    pops.append(begin)

    while True:
        for node in graph[begin][0]:
            if node not in stack and node not in pops and node in graph.keys():
                stack.append(node)

        if len(stack) == 0:
            graph[origin][1] = len(pops)
            break

        begin = stack.pop()

        if begin not in pops:
            pops.append(begin)

def middle(L):
    L = sorted(L)
    n = len(L)
    if n == 0:
        return 0
    m = n - 1
    print m
    print n
    return (L[n/2] + L[m/2]) / 2.0

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print "Usage:", sys.argv[0], "<run time>"
        sys.exit(0)

    sys.setrecursionlimit(4096)

    parse_input()

    for x in range(0, int(sys.argv[1])):
        # Step 1: assign value
        assign_value()

        # Step 2: find the minimum value
        while True:
            min_tuple = find_minimum()
            min_value = min_tuple[0]
            min_key = min_tuple[1]
            # Step 3: update the minmum value
            update_minimum(min_key, min_key, min_value)

            flag = 0
            for k,v in reverse_graph.items():
                if v[2] == 0:
                    flag = 1

            if flag == 1:
                continue

            break

        # Step 4:
        for k,v in reverse_graph.items():
            results[k].append(int(1/v[1]))

    # Calculate the correct value
    for node in graph:
        dfs_degree(node, node)

    keylist = reverse_graph.keys()
    keylist.sort()
    for key in keylist:
        print key, graph[key][1], middle(results[key])

    # for debug
    #print graph
    #print reverse_graph
