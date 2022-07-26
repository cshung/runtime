from readchar import readkey
from functools import cmp_to_key
from os import system

current_committed = 0
animate = False

def cmp_for_add(i, j):
    ai = abs(i)
    aj = abs(j)
    if ai < aj:
        return -1
    elif ai > aj:
        return 1
    elif i < j:
        return 1
    elif i > j:
        return -1
    else:
        return 0

def cmp_for_del(i, j):
    ai = abs(i)
    aj = abs(j)
    if ai < aj:
        return -1
    elif ai > aj:
        return 1
    elif i < j:
        return -1
    elif i > j:
        return 1
    else:
        return 0

def add_interval(memory, oh, start, end, heap_number):
    global current_committed
    current_committed = current_committed + (end - start)
    if heap_number in memory:
        heap = memory[heap_number]
    else:
        heap = {}
        memory[heap_number] = heap
    if oh in heap:
        regions = heap[oh]
    else:
        regions = []
        heap[oh] = regions
    events = regions
    events.append(start)
    events.append(-end)
    events = sorted(events, key=cmp_to_key(cmp_for_add))
    regions.clear()
    level = 0
    overlap_start = 0
    overlap_end = 0
    for i in range(0, len(events)):
        event = events[i]
        if event > 0:
            if level == 0:
                regions.append(event)
            elif level == 1:
                overlap_start = event
            level = level + 1
        else:
            if level == 1:
                regions.append(event)
            level = level - 1
            if level == 1:
                overlap_end = -event
                if overlap_end > overlap_start:
                    raise ValueError("overlapped commit detected")

def del_interval(memory, oh, start, end, heap_number):
    global current_committed
    current_committed = current_committed - (end - start)
    if heap_number in memory:
        heap = memory[heap_number]
    else:
        heap = {}
        memory[heap_number] = heap
    if oh in heap:
        regions = heap[oh]
    else:
        regions = []
        heap[oh] = regions
    events = regions
    events.append(-start)
    events.append(end)
    events = sorted(events, key=cmp_to_key(cmp_for_del))
    regions.clear()
    level = 0
    for event in events:
        if event > 0:
            if level == 0:
                regions.append(event)
            level = level + 1
        else:
            if level == 1:
                regions.append(event)
            level = level - 1

def verify_memory(memory):
    global current_committed
    check_committed = 0
    for heap_number in memory:
        heap = memory[heap_number]
        for oh in heap:
            regions = heap[oh]
            n = len(regions) // 2
            for i in range(0, n):
                start = regions[2 * i]
                end = -regions[2 * i + 1]
                check_committed = check_committed + (end - start)
                if heap_number != -1 or oh != 4:
                    if start % (4 * 1024 * 1024) != 0:
                        raise ValueError("Regions not aligned")
    if check_committed != current_committed:
        raise ValueError("committed value mismatch")

def memory_to_string(memory):
    system("cls")
    s = ""
    for heap_number in memory:
        s = s + ("heap %s" % heap_number)
        s = s + "\n"
        heap = memory[heap_number]
        for oh in heap:
            s = s + ("  oh %s" % oh)
            s = s + "\n"
            regions = heap[oh]
            n = len(regions) // 2
            for i in range(0, n):
                start = regions[2 * i]
                end = -regions[2 * i + 1]
                s = s + ("    [%s, %s)" % (hex(start), hex(end)))
                s = s + "\n"
    return s

def print_memory(memory):
    print(memory_to_string(memory))

def get_range(state, memory):
    global animate
    if animate:
        print_memory(memory)
        readkey()
    heap_number = state["heap_number"]
    oh = state["oh"]
    if heap_number in memory:
        heap = memory[heap_number]
        if oh in heap:
            regions = heap[oh]
            lo = regions[0]
            hi = -regions[-1]
            if "lo" not in state:
                state["lo"] = lo
            else:
                state["lo"] = min(state["lo"], lo)
            if "hi" not in state:
                state["hi"] = hi
            else:
                state["hi"] = max(state["hi"], hi)

def show_relative(state, memory):
    heap_number = state["heap_number"]
    oh = state["oh"]
    lo = state["lo"]
    hi = state["hi"]
    length = hi - lo
    s = ""
    if heap_number in memory:
        heap = memory[heap_number]
        if oh in heap:
            regions = heap[oh]
            n = len(regions) // 2
            for i in range(0, n):
                start = regions[2 * i]
                end = -regions[2 * i + 1]
                start = (start - lo) / length
                end = (end - lo) / length
                s = s + ("    [%s, %s)" % (start, end))
                s = s + "\n"
    print(s)

def analyze_log(lines, state, process_memory):

    global current_committed
    current_committed = 0

    # memory is map from the heap number to a heap
    # a heap is a map from oh to the region list
    # a region list is a list of endpoints, start endpoint is positive, end endpoint is negative
    memory = {}

    n = 0
    interesting = -1
    for line in lines:
        n = n + 1
        # print(n)
        if n == interesting:
            print_memory(memory)
            print(line)
        tokens = line.split()
        if len(tokens) > 0:
            if tokens[1] == "commit":
                oh = int(tokens[3])
                start = int(tokens[4][1:-1],16)
                end = int(tokens[5][:-1],16)
                heap = int(tokens[8])
                add_interval(memory, oh, start, end, heap)
            elif tokens[1] == "decommit":
                oh = int(tokens[3])
                start = int(tokens[4][1:-1],16)
                end = int(tokens[5][:-1],16)
                heap = int(tokens[8])
                del_interval(memory, oh, start, end, heap)
            elif tokens[1] == "from" and tokens[4] == "free":
                oh = int(tokens[2])
                start = int(tokens[5][1:-1],16)
                end = int(tokens[6][:-1],16)
                heap = int(tokens[9])
                del_interval(memory, oh, start, end, heap)
                add_interval(memory, 3, start, end, -1)
            elif tokens[1] == "from" and tokens[2] == "free":
                oh = int(tokens[4])
                start = int(tokens[5][1:-1],16)
                end = int(tokens[6][:-1],16)
                heap = int(tokens[9])
                del_interval(memory, 3, start, end, -1)
                add_interval(memory, oh, start, end, heap)
            elif tokens[1] == "from" and tokens[2] == "temp":
                oh = int(tokens[4])
                start = int(tokens[5][1:-1],16)
                end = int(tokens[6][:-1],16)
                heap = int(tokens[9])
                add_interval(memory, oh, start, end, heap)
            elif tokens[1] == "from" and tokens[4] == "temp":
                oh = int(tokens[2])
                start = int(tokens[5][1:-1],16)
                end = int(tokens[6][:-1],16)
                heap = int(tokens[9])
                del_interval(memory, oh, start, end, heap)
            elif tokens[1] == "checkpoint":
                pass
            else:
                print("Parse error")
                break
        if n == interesting:
            print_memory(memory)
        verify_memory(memory)
        process_memory(state, memory)


def main():
    with (open("D:\\runtime\\andrew.txt")) as f:
        lines = f.readlines()

    # state is simply a useful set of variables for process_memory
    state = {}
    state["heap_number"] = 0
    state["oh"] = 0

    analyze_log(lines, state, get_range)
    # analyze_log(lines, state, show_relative)

if __name__ == "__main__":
    main()