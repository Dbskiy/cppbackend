import argparse
import subprocess
import time
import random
import shlex

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
    return parser.parse_args().server


def run(command, input=None, output=None):
    process = subprocess.Popen(shlex.split(command), stdin=input, stdout=output, stderr=subprocess.DEVNULL)
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


server = run(start_server())
perf_record_cmd = f'perf record -ga -o perf.data'
perf_record = run(perf_record_cmd, output=subprocess.DEVNULL)

make_shots()
stop(server)

time.sleep(1)
stop(perf_record)
perf_record.wait()

perf_script = run("perf script -i perf.data", output=subprocess.PIPE)
stackcollapse_perf = run("./FlameGraph/stackcollapse-perf.pl", input=perf_script.stdout, output=subprocess.PIPE)
perf_script.wait()

with open('graph.svg', 'w') as f:
    flamegraph = run("./FlameGraph/flamegraph.pl", input=stackcollapse_perf.stdout, output=f)
    stackcollapse_perf.wait()
    flamegraph.wait()
print('Job done')
