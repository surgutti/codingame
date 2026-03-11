# Winter Challenge 2026: SNAKEBYTE

[Contest Link](https://www.codingame.com/contests/winter-challenge-2026-exotec)

## Build the referee

This project uses Maven + Java 17.

### Install required packages

For Linux (Debian/Ubuntu):
- `sudo apt update`
- `sudo apt install openjdk-17-jdk maven python3`
- Optional for local bot/frontend work: `nodejs npm`
- On Arch Linux: `sudo pacman -S jdk17-openjdk maven python nodejs npm`

For Windows:
- Install Temurin/OpenJDK 17 and Apache Maven

### Compile to referee.jar

From repo root:

```bash
cd WinterChallenge2026-Exotec
./scripts/build_referee.sh
```

The script runs `mvn -DskipTests package` and copies:

`target/winter-challenge-2025-snakebird-1.0-SNAPSHOT.jar` -> `referee.jar`

The build now produces an executable `referee.jar` with the `CommandLineInterface` entrypoint used by `cg-brutaltester` and other local runners.

### Notes

- `pom.xml` sets Java 17 explicitly (`maven.compiler.release=17`).
- If you are on a machine with limited network, run Maven with access to Maven Central so plugin and dependency downloads can complete.

### Run one test game locally

From the repo root (assuming 2 bots exist):

```bash
java -jar referee.jar -league 1 -p1 "python3 bot1.py" -p2 "python3 bot2.py"
```

You can use `-league 1/2/3/4` for wood/silver/gold/legend difficulty.

### Brutaltester usage

```bash
java -jar cg-brutaltester.jar \
  -r "java --add-opens java.base/java.lang=ALL-UNNAMED -jar referee.jar -league 1" \
  -p1 "python3 bot1.py" \
  -p2 "python3 bot2.py" \
  -n 100 -t 4
```

If your command line for `-r` includes `-s`, it enables server mode in this CLI.
