import pexpect
import argparse

def parse_arguments() -> None:
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
    print(data)
    return
    
    child.expect(">")
    print(child.before)
    
    
    child.sendline("5")
    child.expect(">")
    print(child.before)
    child.sendline(data)
    
    child.expect("Program data")
    print(child.before)



if __name__ == "__main__":
    args = parse_arguments()
    filename = args.program_data;
    with open(filename , "r") as f:
        child = pexpect.spawn('pio device monitor')
        for line in f:
            send_data(line)
