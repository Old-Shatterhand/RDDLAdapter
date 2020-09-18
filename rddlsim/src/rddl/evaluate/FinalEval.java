/** Final Scoring for Competitors (assumes 'min_max_norm_constants.txt' is given
 *  or has already been generated by MinMaxEval).  Outputs 'all_results.txt'.
 *   
 * @author Scott Sanner (ssanner@gmail.com)
 */

package rddl.evaluate;

import java.io.*;
import java.text.DecimalFormat;
import java.util.*;

import javax.xml.namespace.QName;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathExpression;
import javax.xml.xpath.XPathExpressionException;
import javax.xml.xpath.XPathFactory;

import org.apache.xerces.parsers.DOMParser;
import org.w3c.dom.Document;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import rddl.parser.parser;

import util.DocUtils;
import util.MapList;
import util.Pair;

public class FinalEval {
	
	public static int NUM_EXPECTED_TRIALS  = 30;
	public static long TIME_ALLOWED = 1080000;

	public static boolean ENFORCE_TIME_LIMIT = false;

	public static DecimalFormat df = new DecimalFormat("#.##");
	public static DecimalFormat df4 = new DecimalFormat("#.####");

	public static final String IGNORE_CLIENT_LIST_FILE = "IGNORE_CLIENT_LIST.txt";
	public static HashSet<String> IGNORE_POLICIES = new HashSet<String>();

	/**
	 * @param args
	 */	
	public static void Eval(File f) throws Exception {
		
		HashMap<String,MapList> client2data = new HashMap<String,MapList>();
		
		if (f.isDirectory()) {
			
			// Add additional client names to ignore from IGNORE_LIST_FILE
			String s_ignore = DocUtils.ReadFile(new File(f.getPath() + File.separator + IGNORE_CLIENT_LIST_FILE));
			for (String s : s_ignore.split("[\\s]")) {
				IGNORE_POLICIES.add(s.trim());
				System.out.println("[SERVER] Ignoring: '" + s.trim() + "'");
			}

			// Read all log files
			for (File f2 : f.listFiles())
				if (f2.getName().endsWith(".log")) {
					System.out.println("[SERVER] Loading log file: " + f2 + "...");
					LogReader lr = new LogReader(f2);
					System.out.println("[SERVER] " + lr._client2data);
					client2data.putAll(lr._client2data);
				}
		} else
			usage();

		HashMap<String,Double> instance2min = new HashMap<String,Double>();
		HashMap<String,String> instance2minSrc = new HashMap<String,String>();
		HashMap<String,Double> instance2max = new HashMap<String,Double>();
		
		BufferedReader br = new BufferedReader(
				new FileReader(f.getPath() + File.separator + MinMaxEval.MIN_MAX_FILE));
		String line = null;
		while ((line = br.readLine()) != null) {
			String split[] = line.split("\t");
			double min = new Double(split[2]);
			double max = new Double(split[4]);
			instance2min.put(split[0], min);
			instance2minSrc.put(split[0], split[1]);
			instance2max.put(split[0], max);
		}
		br.close();
		
		HashMap<Pair<String,String>,ArrayList<Double>> instance2allrewards = new HashMap<Pair<String,String>,ArrayList<Double>>();
		HashMap<Pair<String,String>,Integer> instance2count   = new HashMap<Pair<String,String>,Integer>();
		HashMap<Pair<String,String>,Double>  instance2minR    = new HashMap<Pair<String,String>,Double>();
		HashMap<Pair<String,String>,Double>  instance2maxR    = new HashMap<Pair<String,String>,Double>();
		HashMap<Pair<String,String>,Double>  instance2avg     = new HashMap<Pair<String,String>,Double>();
		HashMap<Pair<String,String>,Double>  instance2stderr  = new HashMap<Pair<String,String>,Double>();

		TreeSet<String> instances = new TreeSet<String>(new MinMaxEval.InstNameComparator());
		TreeSet<String> domains   = new TreeSet<String>();
		TreeSet<String> clients   = new TreeSet<String>();

		for (Map.Entry<String, MapList> e : client2data.entrySet()) {
			String client_name = e.getKey();
			if (client_name == null) {
				System.err.println("[SERVER] Client name was null for " + e + "... skipping");
				continue;
			} else if (IGNORE_POLICIES.contains(client_name)) {
				System.out.println("[SERVER] Ignoring " + client_name + "... skipping");
				continue;
			}
			clients.add(client_name);
			
			for (Object o : e.getValue().keySet()) {
				String instance_name = (String)o;
				if (instance_name.endsWith("__trial_time"))
					continue;
				instances.add(instance_name);
				domains.add(GetDomainName(instance_name));
				
				Double instance_min = instance2min.get(instance_name);
				if (instance_min == null) {
					System.out.println("[SERVER] ERROR: could not find min for: " + instance_name);
					System.exit(1);
				}

				ArrayList<Double> rewards = new ArrayList<Double>((e.getValue().getValues(instance_name)));
				ArrayList<Long>   times   = new ArrayList<Long>((e.getValue().getValues(instance_name + "__trial_time")));
				
				if (MinMaxEval.BASELINE_POLICIES.contains(client_name.toLowerCase())
						&& rewards.size() != NUM_EXPECTED_TRIALS) {
						System.err.println("[SERVER] INCORRECT NUMBER OF TRIALS [" + rewards.size() + "/"
								+ NUM_EXPECTED_TRIALS + "] for " + client_name + " on " + instance_name);
						System.exit(1);
				}
				
				Pair<String,String> key = new Pair<String,String>(client_name,instance_name);
				instance2count.put(key,  rewards.size());
				instance2allrewards.put(key, rewards);

				///////////////////////////////////////////////////////////////////////////////////////////////////////
				
				// Get up to last FinalEval.NUM_EXPECTED_TRIALS within cumulative time limit of FinalEval.TIME_ALLOWED
				ArrayList<Double> last_rewards_in_trial_and_time_limit = new ArrayList<Double>();
				ArrayList<Long> last_times_in_trial_and_time_limit = new ArrayList<Long>();
				long cumulative_time = 0; 
				for (int i = rewards.size() - 1 /*end*/; i >= Math.max(0, rewards.size() - FinalEval.NUM_EXPECTED_TRIALS) /*e.g., max(end-30,0)*/; i--) {
					
					double rew = rewards.get(i);
					long time  = times.get(i);
					if (ENFORCE_TIME_LIMIT && cumulative_time + time > FinalEval.TIME_ALLOWED) {
						System.err.println("[SERVER] TIME LIMIT (" + (cumulative_time + time) + "/" + FinalEval.TIME_ALLOWED + ") EXCEEDED on " + instance_name +
								           " by " + client_name + ", using last " + last_rewards_in_trial_and_time_limit.size() + " / " + rewards.size() + " trials.");
						break;
					}
					
					last_rewards_in_trial_and_time_limit.add(rew);
					last_times_in_trial_and_time_limit.add(time);
					cumulative_time += time;
				}
				rewards.clear(); // Need to modify in place since external references to this object 
				rewards.addAll(last_rewards_in_trial_and_time_limit); // Replace with the subset within the time limit
				times.clear(); // Need to modify in place since external references to this object
				times.addAll(last_times_in_trial_and_time_limit);
				
//				if (rewards.size() > NUM_EXPECTED_TRIALS) {
//					// Take the last NUM_EXPECTED_TRIALS
//					Object[] temp_rewards = rewards.toArray();
//					rewards.clear();
//					for (int i = temp_rewards.length - NUM_EXPECTED_TRIALS; i < temp_rewards.length; i++)
//						rewards.add((Double)temp_rewards[i]);
//				} else
					
				if (rewards.size() < NUM_EXPECTED_TRIALS) {
					// Use min reward if not enough trials
					System.err.print("[SERVER] - PADDING " + client_name + " / " + instance_name + " WITH MIN REWARD");
					for (int i = rewards.size(); i < NUM_EXPECTED_TRIALS; i++) {
						rewards.add(instance_min);
						System.err.print("[SERVER] .");
					}
					System.err.println();
				}

				if (rewards.size() != NUM_EXPECTED_TRIALS) {
					System.out.println("[SERVER] INCORRECT NUMBER OF TRIALS [" + rewards.size() + "/"
							+ NUM_EXPECTED_TRIALS + "] for " + client_name + " after correction.");
					System.exit(1);
				}
				///////////////////////////////////////////////////////////////////////////////////////////////////////

				double avg = Statistics.Avg(rewards);
				double stderr = Statistics.StdError95(rewards);

				instance2minR.put(key,   Statistics.Min(rewards));
				instance2maxR.put(key,   Statistics.Max(rewards));
				instance2avg.put(key,    avg);
				instance2stderr.put(key, stderr);
			}
		}
		
		// TODO: Show details like variable count for domains, largest CPT, tree width, etc?
		//       Probably another domain analysis file.
		
		// TODO: Track aggregate results per instance... remove __#

		PrintStream all_results = new PrintStream(new FileOutputStream(f.getPath() + File.separator + "all_results.txt"));

		System.out.println("[SERVER] All results\n===");
		all_results.println("[SERVER] All results\n===");
		
		MapList client2normval = new MapList();
		MapList client2normvalAll = new MapList();
		MapList domain_client2normval = new MapList();
		MapList domain_client2normvalAll = new MapList();
		for (String instance_name : instances) {
			
			String domain_name = GetDomainName(instance_name);
			double instance_min = instance2min.get(instance_name);
			double instance_max = instance2max.get(instance_name);
			all_results.print(instance_name + "\t");
			System.out.print("[SERVER] " + instance_name + "\t");
			
			for (String client_name : clients) {
				Pair<String,String> key = new Pair<String,String>(client_name,instance_name);
				
				Integer count = instance2count.get(key);
				if (count == null) {
					String min_client_name = instance2minSrc.get(instance_name);
					key = new Pair<String,String>(min_client_name,instance_name);
					
					count = 0;//instance2count.get(key);
				}
				ArrayList<Double> all_rewards = instance2allrewards.get(key);
				double min_val = instance2minR.get(key);
				double max_val = instance2maxR.get(key);
				double avg     = instance2avg.get(key);
				double stderr  = instance2stderr.get(key);
				
				// Don't allow scores below instance_min
				double pre_min_avg = avg; 
				if (avg < instance_min)
					avg = instance_min;

				double range = instance_max - instance_min;
				if (range == 0d)
					range = 1e10d;
				double norm_score = (avg - instance_min) / range;
				client2normval.putValue(client_name, norm_score);
				domain_client2normval.putValue(new Pair<String,String>(domain_name,client_name), norm_score);
				
				// Note: the "min-average rule" technically prevents us from directly normalizing individual
				//       performances -- the instance normalized avg is the result of a min function
				for (Double reward : all_rewards) {
					double norm_reward = (reward - instance_min) / range;
					client2normvalAll.putValue(client_name, norm_reward);
					domain_client2normvalAll.putValue(new Pair<String,String>(domain_name,client_name), norm_reward);
				}
				
				System.out.print("[SERVER] "client_name + "\t" + count + "\t" + format4(norm_score) + "\t" + format(pre_min_avg) + "\t+/- " + format(stderr) + "\t[ " + format(min_val) + "\t" + format(max_val) + " ]\t");
				all_results.print(client_name + "\t" + count + "\t" + format4(norm_score) + "\t" + format(pre_min_avg) + "\t" + format(stderr) + "\t" + format(min_val) + "\t" + format(max_val) + "\t");
			}
			System.out.println();
			all_results.println();
		}
		
		System.out.println("\n[SERVER] Avg of all normalized rewards by domain\n===");
		all_results.println("\nAvg of all normalized rewards by domain\n===");
		for (String domain_name : domains) {
			System.out.print("[SERVER] " + domain_name + "\t");
			all_results.print(domain_name + "\t");
			for (String client_name : clients) {
				ArrayList<Double> norm_scoresAll = (ArrayList<Double>)domain_client2normvalAll.getValues(new Pair<String,String>(domain_name,client_name));
				double avgAll = Statistics.Avg(norm_scoresAll);
				double stderrAll = Statistics.StdError95(norm_scoresAll);
				System.out.print("[SERVER] \t" + client_name + "\t" + norm_scoresAll.size() + "\t" + format4(avgAll) + "\t+/- " + format4(stderrAll));
				all_results.print("\t" + client_name + "\t" + norm_scoresAll.size() + "\t" + format4(avgAll) + "\t" + format4(stderrAll));
			}
			System.out.println();
			all_results.println();
		}
		
		System.out.println("\n[SERVER] Avg of min(0,avg-norm-score-instance)\n===");
		all_results.println("\nAvg of min(0,avg-norm-score-instance)\n===");
		for (String domain_name : domains) {
			System.out.print("[SERVER] " + domain_name + "\t");
			all_results.print(domain_name + "\t");
			for (String client_name : clients) {
				ArrayList<Double> norm_scores = (ArrayList<Double>)domain_client2normval.getValues(new Pair<String,String>(domain_name,client_name));
				double avg = Statistics.Avg(norm_scores);
				double stderr = Statistics.StdError95(norm_scores);
				System.out.print("[SERVER] \t" + client_name + "\t" + norm_scores.size() + "\t" + format4(avg) + "\t+/- " + format4(stderr));
				all_results.print("\t" + client_name + "\t" + norm_scores.size() + "\t" + format4(avg) + "\t" + format4(stderr));
			}
			System.out.println();
			all_results.println();
		}

		System.out.println("\n[SERVER] Avg of all normalized rewards\n===");
		all_results.println("\nAvg of all normalized rewards\n===");
		for (String client_name : clients) {
			ArrayList<Double> norm_scoresAll = (ArrayList<Double>)client2normvalAll.getValues(client_name);
			double avgAll = Statistics.Avg(norm_scoresAll);
			double stderrAll = Statistics.StdError95(norm_scoresAll);
			System.out.println("[SERVER] " + client_name + "\t" + norm_scoresAll.size() + "\t" + format4(avgAll) + "\t+/- " + format4(stderrAll));
			all_results.println(client_name + "\t" + norm_scoresAll.size() + "\t" + format4(avgAll) + "\t" + format4(stderrAll));
		}

		System.out.println("\n[SERVER] Avg of min(0,avg-norm-score-instance)\n===");
		all_results.println("\n Avg of min(0,avg-norm-score-instance)\n===");
		for (String client_name : clients) {
			ArrayList<Double> norm_scores = (ArrayList<Double>)client2normval.getValues(client_name);
			double avg = Statistics.Avg(norm_scores);
			double stderr = Statistics.StdError95(norm_scores);
			System.out.println("[SERVER] " + client_name + "\t" + norm_scores.size() + "\t" + format4(avg) + "\t+/- " + format4(stderr));
			all_results.println(client_name + "\t" + norm_scores.size() + "\t" + format4(avg) + "\t" + format4(stderr));
		}
		
		all_results.close();
	}
	
	public static String format(Double d) {
		if (d == null)
			return null;
		else
			return df.format(d);
	}
	
	public static String format4(Double d) {
		if (d == null)
			return null;
		else
			return df4.format(d);
	}
	
	public static String GetDomainName(String instance_name) {
		String split[] = instance_name.split("__");
		String domain_name = split[0];
		return domain_name.replace("_inst", "");
	}
	
	public static int ProcessArgs(String[] args, int index) {
		while (args[index].startsWith("--")) {
			if (args[index].equalsIgnoreCase("--time-limit")) {
				TIME_ALLOWED = new Long(args[index + 1]);
				ENFORCE_TIME_LIMIT = true;
				index += 2;
			} else if (args[index].equalsIgnoreCase("--num-trials")) {
				NUM_EXPECTED_TRIALS = new Integer(args[index + 1]);
				index += 2;
			} else {
				System.out.println("[SERVER] Unrecognized option '" + args[index] + " " + args[index + 1] + "'");
				usage();
			}
		}
		return index;
	}
	
	public static void main(String[] args) throws Exception {
		
		String directory = null;
		int index = ProcessArgs(args, 0);
		if (index != args.length - 1)
			usage();
		directory = args[index];
		
		Eval(new File(directory));
	}

	public static void usage() {
		System.out.println("\n[SERVER] Usage: [--time-limit TIME] [--num-trials NUM] <directory of RDDL .log files>");
	}
}
