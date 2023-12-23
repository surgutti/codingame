package benchmarker;

public abstract class Confidence {

  public static double getConfidence95Interval(int wins, int draws, int loses) {
    int plays = wins + draws + loses;
    double avg = (wins + draws * 0.5) / plays;
    double var = (wins + draws * 0.25 - avg * avg * plays) / (wins+draws+loses-1);
    return 1.9602 * Math.sqrt(var / (plays - 1));// 95%
  }

  public static double getConfidence99Interval(int wins, int draws, int loses) {
    int plays = wins + draws + loses;
    double avg = (wins + draws * 0.5) / plays;
    double var = (wins + draws * 0.25 - avg * avg * plays) / (wins+draws+loses-1);
    return 2.5763 * Math.sqrt(var / (plays - 1));// 99%
  }

}
