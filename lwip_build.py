
from pathlib import Path

Import("env")

project = Path(env.subst("$PROJECT_DIR"))
lwip = project / "Middlewares" / "Third_Party" / "LwIP"

source_dirs = [
    lwip / "src" / "core",
    lwip / "src" / "core" / "ipv4",
    lwip / "src" / "netif",
    lwip / "src" / "api",
]

for folder in source_dirs:
    if folder.exists():
        objects = env.BuildSources(
            str(project / ".pio" / "lwip_build" / folder.relative_to(lwip)),
            str(folder),
            "*.c"
        )
        env.Append(PIOBUILDFILES=objects)

