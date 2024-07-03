import argparse
import subprocess
import time
import random
import shlex

#comment

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

def generate_flamegraph():
    flamegraph_cmd = 'sudo perf script | ./FlameGraph/stackcollapse-perf.pl | ./FlameGraph/flamegraph.pl > graph.svg'
    subprocess.run(flamegraph_cmd, shell=True)

server_cmd = start_server()
server = run(server_cmd)
time.sleep(5) 

perf_record_cmd = f'sudo perf record -o perf.data -p {server.pid}'
perf_record = subprocess.Popen(shlex.split(perf_record_cmd))

make_shots()

stop(server)
time.sleep(1)

perf_record.terminate()
time.sleep(1)

generate_flamegraph()

print('Job done')
