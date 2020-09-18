package rddl.translate;

import java.io.File;
import java.io.FileOutputStream;
import java.io.PrintStream;
import java.util.ArrayList;
import java.util.Arrays;

import rddl.RDDL;
import rddl.parser.parser;

public class RDDL2Prefix {

	/**
	 * @param args
	 */
	public static void main(String[] args) throws Exception {
		
		RDDL.USE_PREFIX = true;
	
		if (args.length != 2) {
			System.out.println("\n[SERVER] usage: RDDL-file/directory output-dir\n");
			System.exit(1);
		}
		int arg_offset = 0;

		// Since fluents in prefix format will always be surrounded by parens and object fluents will not, I believe
		// that it will be unambiguous to always suppress the dollar sign in prefix format, so I will make this default in the RDDL
		// toString() method for prefix-output rather than making it optional (as the commented out code below was intended to do)
		
		String rddl_file = args[arg_offset];
		String output_dir = args[arg_offset + 1];
				
		if (output_dir.endsWith(File.separator))
			output_dir = output_dir.substring(output_dir.length() - 1);
		
		// Create output dir if it does not exist
		File output_dir_file = new File(output_dir);
		if (!output_dir_file.exists())
			output_dir_file.mkdir();
		
		// If RDDL file is a directory, add all files
		ArrayList<File> rddl_files = new ArrayList<File>();
		ArrayList<File> rddl_error = new ArrayList<File>();
		File file = new File(rddl_file);
		if (file.isDirectory())
			rddl_files.addAll(Arrays.asList(file.listFiles()));
		else
			rddl_files.add(file);
		
		for (File f : (ArrayList<File>)rddl_files.clone()) {
						
			try {
				RDDL rddl = parser.parse(f);
				
				// Could suppress $ from RDDL2 if there are no object fluents, but this could cause confusion for RDDL2 users, so omitting for now 
				//if (!rddl.containsObjectFluents()) 
				//	RDDL.SUPPRESS_OBJECT_CAST = true;
				
				int rddl_index = f.getName().indexOf(".rddl");
				File output_file = new File(output_dir + File.separator + 
						f.getName().substring(0, rddl_index) + ".rddl_prefix");
				System.out.println("[SERVER] Writing to " + output_file);
				
				PrintStream ps = new PrintStream(new FileOutputStream(output_file));
				ps.print(rddl.toString());
				ps.close();
			
			} catch (Exception e) {
				System.err.println("[SERVER] Error processing: " + f);
				System.err.println("[SERVER] " + e);
				e.printStackTrace(System.err);
				System.err.flush();
				rddl_files.remove(f);
				rddl_error.add(f);
			}
		}
		
		System.out.println("\n\n[SERVER] ===\n");
		for (File f : rddl_files)
			System.out.println("[SERVER] Processed: " + f);
		for (File f : rddl_error)
			System.out.println("[SERVER] Error processing: " + f);
	}

}
