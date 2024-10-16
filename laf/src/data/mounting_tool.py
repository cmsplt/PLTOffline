import os
import sys


class MountData:
    """
    Utility class to mount the data from the PLT-BRIL detector. 

    The entered user and password are used to authenticate the 
    connection to the cmsusr and brildata machines.

    Args:
        user (str): User to authenticate the connection.
        password (str, optional): Password to authenticate the connection. Defaults to None.

    Returns:None
    """
    def __init__(self, user: str,
                 password: str = None):
        self.user = user
        self.password = password

    def create_mount(self, mount_source: str, mount_target: str) -> None:
        """This function creates a mount between a source and a target by 
        by performung a double ssh to the cmusr and brilmachine.

        Args:
            mount_source (str, optional): Machine where data is stored.
            mount_target (str, optional): Path to where the mount is going
            to be created. If it already exists, no exception is raised and if
            the path does not exist, it is created.
        Returns:
            None: Nothing is returned.
        
        Example:
            >>> mount_source = "brildev1:/brildata/22/"
            >>> mount_target = "./Files/22"
            >>> m = MountData("YOUR USER", "YOUR PASSWORD")
            >>> m.create_mount(mount_source, mount_target)
        
        """
        os.system(self._mount_command(self.user,
                                    self.password,
                                    mount_source,
                                    mount_target))
        

    def _mount_command(self, user,
                      password,
                      mount_source="brildev1:/brildata/22/",
                      mount_target="./Files/22"):
        """This function creates a mount string to be excecuted
           as a system command."""
        # if mount source is not a directory, create it
        if not os.path.isdir(mount_target):
            print("Mount target is not a directory. Creating it...")
            os.makedirs(mount_target)
        return f"""sshfs -o reconnect -o password_stdin \
                -oProxyCommand="ssh -W %h:%p {user}@cmsusr.cern.ch" \
                {mount_source} {mount_target} <<< '{password}'"""


