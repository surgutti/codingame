package benchmarker;

import com.codingame.gameengine.runner.MultiplayerGameRunner;
import com.codingame.gameengine.runner.simulate.GameResult;

import java.io.*;
import java.util.StringTokenizer;

/**
 * By MSz 2023
 */
public class WorkerProcess extends Thread {

  private static GameResult runOnePlay(String agent1, String agent2, long seed) {
    // TODO Fix static variables
    MultiplayerGameRunner gameRunner = new MultiplayerGameRunner();
    // TODO Set league level
    gameRunner.setLeagueLevel(4);
    gameRunner.addAgent(agent1);
    gameRunner.addAgent(agent2);
    gameRunner.setSeed(seed);
    return gameRunner.simulate();
  }

  /**
   * In a separate process
   */
  public static void main(String[] params) throws Exception {
    if (params.length != 1 || !params[0].equals("worker"))
      throw new Exception("This is not for manual run!");
    try {
      BufferedReader input = new BufferedReader(new InputStreamReader(System.in));
      String line;
      while ((line = input.readLine()) != null) {
        if (line.equals("q")) return;
        StringTokenizer args = new StringTokenizer(line, "|");
        String agent1 = args.nextToken(), agent2 = args.nextToken();
        long seed = Long.parseLong(args.nextToken());
        try {
          GameResult gameResult = runOnePlay(agent1, agent2, seed);
//// May be useful for debug in case of agent errors:
//          if (gameResult.scores.get(0) == -1 || gameResult.scores.get(1) == -1) {
//            throw new Exception(gameResult.summaries.toString());
//          }
          System.out.println(gameResult.scores.get(0) + "|" + gameResult.scores.get(1));
        } catch (Exception e) {
          System.out.println(e);
          System.out.flush();
          continue;
        }
        System.out.flush();
      }
    } catch (Exception e) {
      System.out.println(e);
      System.out.flush();
    }
  }

  // ******************************************************************************************************************

  static final String JAVA_BIN = System.getProperty("java.home") + File.separator + "bin" + File.separator + "java";
  public static final String[] MAIN_CMD = new String[]{
    JAVA_BIN,
    "--add-opens", "java.base/java.lang=ALL-UNNAMED",
    "-cp", System.getProperty("java.class.path"),
    WorkerProcess.class.getName(),
    "worker"
  };

  public static Process exec() throws IOException {
    return Runtime.getRuntime().exec(MAIN_CMD);
  }

  // ******************************************************************************************************************

  final Benchmarker main;
  Process workerProcess;
  BufferedWriter processInput;
  BufferedReader processOutput;

  WorkerProcess(Benchmarker main) {
    this.main = main;
  }

  void startProcess() throws IOException {
    workerProcess = WorkerProcess.exec();
    processInput = new BufferedWriter(new OutputStreamWriter(workerProcess.getOutputStream()));
    processOutput = new BufferedReader(new InputStreamReader(workerProcess.getInputStream()));
  }

  void closeProcess() throws IOException, InterruptedException {
    if (workerProcess == null) return;
    try {
      processInput.write("q\n");
      processInput.close();
      workerProcess.waitFor();
    } finally {
      workerProcess = null;
    }
  }

  @Override public void run() {
    try {
      startProcess();
      while (true) {
        Benchmarker.Task task = main.taskQueue.take();
        if (task.seed == 0) break;
        String agent1 = Benchmarker.AGENTS_PATH + Benchmarker.AGENT1;
        String agent2 = Benchmarker.AGENTS_PATH + Benchmarker.AGENT2;
        if (task.reverse)
          processInput.write(agent2 + "|" + agent1 + "|" + task.seed + "\n"); else
          processInput.write(agent1 + "|" + agent2 + "|" + task.seed + "\n");
        processInput.flush();
        String outStr = processOutput.readLine();
        StringTokenizer outDesc = new StringTokenizer(outStr, "|");
        if (outDesc.countTokens() != 2) throw new Exception(outStr);
        int[] scores = new int[]{Integer.parseInt(outDesc.nextToken()), Integer.parseInt(outDesc.nextToken())};
        main.reportResult(task, scores);
      }
    } catch (Exception e) {
      e.printStackTrace();
    }
    try {
      closeProcess();
    } catch (Exception ignored) {}
  }

}
