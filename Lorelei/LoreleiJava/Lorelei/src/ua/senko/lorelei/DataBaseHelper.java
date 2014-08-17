package ua.senko.lorelei;

import java.io.*;

import android.content.Context;
import android.database.*;
import android.database.sqlite.*;
import android.util.Log;

public class DataBaseHelper extends SQLiteOpenHelper
{
	 
    //The Android's default system path of your application database.
    public static String DB_PATH = "/data/data/ua.senko.lorelei/databases/";

    private final Context myContext;
    
    private String _db_name;
 
    /**
     * Constructor
     * Takes and keeps a reference of the passed context in order to access to the application assets and resources.
     * @param context
     */
    public DataBaseHelper(Context context, String db_name) 
    {
    	super(context, db_name, null, 1);
    	_db_name = db_name;
        this.myContext = context;
    }	
 
  /**
     * Creates a empty database on the system and rewrites it with your own database.
     * */
    public void createDataBase() throws IOException{
 
    	boolean dbExist = checkDataBase();
 
    	if(dbExist){
    		//do nothing - database already exist
    	}else{
 
    		//By calling this method and empty database will be created into the default system path
               //of your application so we are gonna be able to overwrite that database with our database.
        	this.getReadableDatabase();
 
        	try {
 
    			copyDataBase(_db_name);
 
    		} catch (IOException e) {
 
        		throw new Error("Error copying database");
 
        	}
    	}
 
    }
 
    /**
     * Check if the database already exist to avoid re-copying the file each time you open the application.
     * @return true if it exists, false if it doesn't
     */
    private boolean checkDataBase(){
    	Log.d("Lorelei", "Checking DB");
    	return new File(DB_PATH + _db_name).exists();
    }
 
    /**
     * Copies your database from your local assets-folder to the just created empty database in the
     * system folder, from where it can be accessed and handled.
     * This is done by transfering bytestream.
     * */
    private void copyDataBase(String db_name) throws IOException{
 
    	//Open your local db as the input stream
    	InputStream myInput = myContext.getAssets().open(db_name);
 
    	// Path to the just created empty db
    	String outFileName = DB_PATH + _db_name;
 
    	//Open the empty db as the output stream
    	new File(DB_PATH).mkdir();
    	OutputStream myOutput = new FileOutputStream(outFileName);
 
    	//transfer bytes from the inputfile to the outputfile
    	byte[] buffer = new byte[1024];
    	int length;
    	while ((length = myInput.read(buffer))>0){
    		myOutput.write(buffer, 0, length);
    	}
 
    	//Close the streams
    	myOutput.flush();
    	myOutput.close();
    	myInput.close();
 
    }
 
    public SQLiteDatabase openDataBase() throws SQLException 
    {
    	//Open the database
        String myPath = DB_PATH + _db_name;
        if(!checkDataBase()) 
        {
        	Log.d("Lorelei", "Coping DB");
			try {
				copyDataBase(_db_name);
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
	        	Log.d("Lorelei", "Failed to copy DB");
			}
        }
    	return SQLiteDatabase.openDatabase(myPath, null, SQLiteDatabase.OPEN_READONLY); 
    }
 
    @Override
	public synchronized void close() 
    { 
    	    super.close(); 
	}
 
	@Override
	public void onCreate(SQLiteDatabase db) {
 
	}
 
	@Override
	public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
 
	}
        // Add your public helper methods to access and get content from the database.
       // You could return cursors by doing "return myDataBase.query(....)" so it'd be easy
       // to you to create adapters for your views.
 
}