from src.data.mount_fills import MountData
import click


@click.command()
@click.option("--username", prompt="Your username", help="Your username of cmsusr")
@click.option(
    "--password",
    prompt="Your password",
    hide_input=True,
    help="Your password of cmsusr",
)
@click.option(
    "--year", default=22, prompt="Year to mount", help="Year to mount, default is 22"
)
@click.option(
    "--target_dir",
    default="./mounted_fills",
    prompt="Target directory",
    help="Target directory to mount, default is ./mounted_fills",
)
def main(username, password, year, target_dir):
    mounter = MountData(user=username, password=password)
    mounter.mount_fills("brildev1:/brildata/{year}/", target_dir)


if __name__ == "main":
    main()
