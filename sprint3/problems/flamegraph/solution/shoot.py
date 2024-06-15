import argparse
import subprocess
import time
import random
import shlex
import os

RANDOM_LIMIT = 1000
SEED = 123456789
random.seed(SEED)

AMMUNITION = [
    'localhost:8080/api/v1/maps/map1',
    'localhost:8080/api/v1/maps'
]

SHOOT_COUNT = 100
COOLDOWN = 0.1

def start_server():
    parser = argparse.ArgumentParser()
    parser.add_argument('server', type=str)
    args = parser.parse_args()
    return args.server

def run(command, output=None):
    process = subprocess.Popen(shlex.split(command), stdout=output, stderr=subprocess.DEVNULL)
    return process

def stop(process, wait=False):
    if process.poll() is None and wait:
        process.wait()
    process.terminate()

def shoot(ammo):
    hit = run('curl ' + ammo, output=subprocess.DEVNULL)
    time.sleep(COOLDOWN)
    stop(hit, wait=True)

def make_shots():
    for _ in range(SHOOT_COUNT):
        ammo_number = random.randrange(RANDOM_LIMIT) % len(AMMUNITION)
        shoot(AMMUNITION[ammo_number])
    print('Shooting complete')

def main():
    stackcollapse_script = 'home/dbskiy/FlameGraph/stackcollapse-perf.pl'
    flamegraph_script = 'home/dbskiy/FlameGraph/flamegraph.pl'

    server_command = start_server()
    server_process = run(server_command)

    perf_data_file = 'perf.data'
    perf_command = f"perf record -o {perf_data_file} -F 99 -p {server_process.pid} -g --call-graph dwarf"
    perf_process = run(perf_command)

    try:
        make_shots()
    finally:
        stop(perf_process, wait=True)

    if perf_process.returncode == 0:
        os.system(f"perf script -i {perf_data_file} | {stackcollapse_script} | {flamegraph_script} > graph.svg")
        print('FlameGraph is created: graph.svg')
    else:
        print("perf record wasn't executed successfully.")

    stop(server_process)
    time.sleep(1)
    print('Job done')

if __name__ == "__main__":
    main()
