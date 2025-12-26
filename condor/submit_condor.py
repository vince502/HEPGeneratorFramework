#!/usr/bin/env python3
import os
import subprocess
import argparse

def main():
    parser = argparse.ArgumentParser(description='Submit HEP jobs to HTCondor')
    parser.add_argument('--total-events', type=int, default=1000000, help='Total events to generate')
    parser.add_argument('--events-per-job', type=int, default=100000, help='Events per job')
    parser.add_argument('--output-prefix', type=str, default='output_cp5', help='Prefix for output files')
    args = parser.parse_args()

    # Create logs directory
    if not os.path.exists('condor/logs'):
        os.makedirs('condor/logs')

    # Create output directory
    if not os.path.exists('condor/output'):
        os.makedirs('condor/output')

    num_jobs = (args.total_events + args.events_per_job - 1) // args.events_per_job
    
    print(f"Generating {args.total_events} events across {num_jobs} jobs...")
    
    # Generate the queue parameters
    queue_file = "condor/queue_list.txt"
    with open(queue_file, "w") as f:
        f.write("Events, Seed, OutFile\n")
        for i in range(num_jobs):
            seed = 1000 + i
            # Correct path for inside the container/worker node
            # HTCondor transfers to current working dir, so just use filename
            outfile = f"output_{i}.txt"
            f.write(f"{args.events_per_job}, {seed}, {outfile}\n")

    # Add the queue command to a temporary .sub file
    sub_file = "condor/temp_production.sub"
    with open("condor/production.sub", "r") as f:
        common_sub = f.read()
    
    with open(sub_file, "w") as f:
        f.write(common_sub)
        f.write(f"\n# Queue jobs\n")
        f.write(f"queue Events, Seed, OutFile from {queue_file}\n")

    # Submit
    try:
        subprocess.run(["condor_submit", sub_file], check=True)
        print("Successfully submitted to HTCondor!")
    except Exception as e:
        print(f"Submit failed: {e}")
        print(f"You can try manually: condor_submit {sub_file}")

if __name__ == "__main__":
    main()
