package benchmarker;

import org.apache.commons.lang3.StringUtils;

import java.util.Locale;
import java.util.concurrent.ArrayBlockingQueue;

/**
 * By MSz (& JKo) 2023
 */
public class Benchmarker {
  static int NUM_THREADS = 4;
  static String AGENTS_PATH = "./agents/";
  static String AGENT1 = "v2";
  static String AGENT2 = "v1";
  static final long BASE_SEED = 1;
  static long NUM_PLAYS = 100;

  ArrayBlockingQueue<Task> taskQueue = new ArrayBlockingQueue<>(NUM_THREADS);
  WorkerProcess[] workerProcesses = new WorkerProcess[NUM_THREADS];
  int wins, draws, loses, errors;

  public synchronized void reportResult(Task task, int[] scores) {
    String agentA, agentB;
    int scoreA = scores[0], scoreB = scores[1];
    // A is the first player, B is the second
    if (task.reverse) {
      agentA = AGENT2; agentB = AGENT1;
      if (scoreA > scoreB) loses++; else if (scoreA < scoreB) wins++; else draws++;
    } else {
      agentA = AGENT1; agentB = AGENT2;
      if (scoreA > scoreB) wins++; else if (scoreA < scoreB) loses++; else draws++;
    }
    if (scoreA < 0 || scoreB < 0) {
      if (scoreA < 0) System.err.println("Error of " + agentA + " for seed " + task.seed + " (score " + scoreA + ")");
      if (scoreB < 0) System.err.println("Error of " + agentB + " for seed " + task.seed + " (score " + scoreB + ")");
      errors++;
    }
    System.out.println("seed=" + task.seed + " " + agentA + " vs " + agentB + ": " + scoreA  + " vs " + scoreB);
  }

  static class Task {
    final boolean reverse;
    final long seed;
    Task() {seed=0;reverse=false;}
    Task(boolean reverse, long seed) {
      this.reverse = reverse;
      this.seed = seed;
    }
  }

  void run() throws InterruptedException {
    long startTime = System.currentTimeMillis();
    for (int i = 0; i < NUM_THREADS; i++) {
      workerProcesses[i] = new WorkerProcess(this);
      workerProcesses[i].start();
    }

    // For non-symmetric plays:
    //for (int i = 0; i < NUM_PLAYS; i++) taskQueue.put(new Task(false, BASE_SEED + i));
    // For symmetric plays:
    for (int i = 0; i < NUM_PLAYS/2; i++) {
      taskQueue.put(new Task(false, BASE_SEED + i));
      taskQueue.put(new Task(true, BASE_SEED + i));
    }

    for (int i = 0; i < NUM_THREADS; i++) taskQueue.put(new Task());
    for (int i = 0; i < NUM_THREADS; i++) workerProcesses[i].join();

    double score = Math.round(100.0 * (wins + draws * 0.5) / NUM_PLAYS);
    String info = "Summary: " + (int)score + "%";
    double confidenceInterval = Math.round(1000.0 * Confidence.getConfidence95Interval(wins, draws, loses)) / 10.0;
    info += "  ±" + StringUtils.rightPad(String.format(Locale.US, "%.2f", confidenceInterval), 5);
    info += "   " + wins + " : " + draws + " : " + loses;
    info += "  (" + errors + " errors)";
    info += "  " + NUM_PLAYS + " games";
    info += " completed in " + (System.currentTimeMillis()-startTime) / 1000.0 + "s";
    String res = (score-confidenceInterval>50?"WON":(score+confidenceInterval<50)?"LOST":"DRAW");
    System.out.printf("Summary: %s %s vs %s \n", AGENT1, res, AGENT2);
    System.out.println(info);
  }

  public static void main(String[] args) throws InterruptedException {
    if (args.length > 0) AGENT1 = args[0];
    if (args.length > 1) AGENT2 = args[1];
    if (args.length > 2) NUM_PLAYS = Long.parseLong(args[2]);
    if (args.length > 3) NUM_THREADS = Integer.parseInt(args[3]);
    new Benchmarker().run();
  }
}
