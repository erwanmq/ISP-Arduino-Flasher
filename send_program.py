import pexpect
import argparse

def parse_arguments():
    """
    Parse command-line arguments.
    Returns:
        argparse.Namespace: Parsed arguments.
    """
    parser = argparse.ArgumentParser(
        description="A script with help and program-data arguments."
    )

    # Add the 'program-data' argument
    parser.add_argument("--program-data", type=str, required=True, help="A string argument named 'program-data'.")
    

    # Parse the arguments
    args = parser.parse_args()
    return args

def send_data(data: str) -> None:
    child = pexpect.spawn('pio device monitor')
    
    child.expect(">")
    print("Program connected")
    print(child.before)
    
    
    child.sendline("5")
    child.expect(">")
    print(child.before)
    child.sendline(data)
    
    child.expect("Program data")
    print(child.before)



if __name__ == "__main__":
    args = parse_arguments()
    send_data(args.program_data)
