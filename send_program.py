import pexpect
import argparse
import re

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
    parser.add_argument("--program-data", type=str, required=False, help="Pass the ihx file to write to the MCU.")
    parser.add_argument("--verify-data", type=str, required=False, help="Pass the ihx file to verify from the MCU memory.")
    

    # Parse the arguments
    args = parser.parse_args()
    return args

def int_to_2bytes_hex(value: int) -> str:
    """
    Convert an integer to a 2-byte hexadecimal string.

    Args:
        value (int): The integer to convert.

    Returns:
        str: The 2-byte hexadecimal string representation of the input.

    Raises:
        ValueError: If the input value is not in the range [0, 65535].
    """
    if not 0 <= value <= 65535:
        raise ValueError("Input value must be between 0 and 65535 (inclusive).")
    return f"{value:04X}"


def get_display_memory_values(pexpect_output: str) -> list[str]:
    """
    Search for memory pattern (XXXX=Y) in a pexpect output string.

    Args:
        pexpect_output (str): The pexpect output string to search in.

    Returns:
        list[str]: List of all found memory patterns.
    """
    pattern = r"[0-9A-F]{4}=[0-9A-F]+"
    return re.findall(pattern, pexpect_output)


def verify_data(data: str) -> None:
    address = data[3:7]
    child.sendline("6")

    child.expect(">")
    child.sendline(address[0:2])
    print(f"sending {address[0:2]}")
    child.expect(">")
    child.sendline(address[2:4])
    print(f"sending {address[2:4]}")
    child.expect(">")

    size = data[1:3]
    end_address = int_to_2bytes_hex(int(address, 16) + int(size, 16) - 1)
    child.sendline(end_address[0:2])
    print(f"sending {end_address[0:2]}")
    child.expect(">")
    child.sendline(end_address[2:4])
    print(f"sending {end_address[2:4]}")

    child.expect(">")
    memory = get_display_memory_values(str(child.before)) # Get only the address=memory values

    displayed_address, displayed_memory = memory[0].split("=")

    print("\n\n")
    if displayed_memory != data[9:-3]:
        print(f"The data failed to verify.\nWanted: {data[9:-3]} - Found: {displayed_memory}")
    else: 
        print(f"The data are verified.\nWanted: {data[9:-3]} - Found: {displayed_memory}")
    
    print("\n\n")

    

def send_data(data: str) -> None:

    print(f"Sending {line}...")
    child.sendline("5")
    child.expect(">")
    child.sendline(data)
    
    child.expect(">")




if __name__ == "__main__":
    args = parse_arguments()
    filename = args.program_data
    if filename is not None:
        child = pexpect.spawn('pio device monitor')
        child.expect(">")
        child.sendline("3")
        child.expect(">")
        print("Chip full erased!")
        with open(filename , "r") as f:
            for line in f:
                send_data(line)
        child.sendline("7")
        print("MCU ready to be used!")

    filename = args.verify_data
    if filename is not None:
        with open(filename , "r") as f:
            child = pexpect.spawn('pio device monitor')
            child.expect(">")
            for line in f:
                verify_data(line)

