package ua.senko.lorelei;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.*;

public class MainActivity extends Activity {

	Lorelei lora;
	
	EditText loreleiInput;
	TextView loreleiOutput;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		lora = new Lorelei(this);
		
		lora.Init();
		
		loreleiInput = (EditText)findViewById(R.id.loreleiInput);
		loreleiOutput = (TextView)findViewById(R.id.loreleiOutput);
		
		final Button exec = (Button)findViewById(R.id.execButton);
				
		lora.SetPrinter(new IPrinter() {
			
			@Override
			public void PrintError(String err_str) {
				loreleiOutput.append("Error: " + err_str);
			}
			
			@Override
			public void PrintDebugLog(String log_str) {
				loreleiOutput.append(log_str);
			}
		});
		
		exec.setOnClickListener(new View.OnClickListener() {			
			@Override
			public void onClick(View v) 
			{
				lora.Parse(loreleiInput.getText().toString());
				
			    lora.PrintResults();

			    lora.ClearCommands();
			}
		});		
	}
	
	@Override
	protected void onDestroy() 
	{
		super.onDestroy();
		lora.Close();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		int id = item.getItemId();
		switch (id) {
		case R.id.action_clear:
			loreleiOutput.setText("");
			break;
		case R.id.action_about:
			AlertDialog.Builder builder = new AlertDialog.Builder(MainActivity.this);
			builder.setTitle(R.string.action_about)
					.setMessage("Version: " + lora.GetVersion() + 
							"\nDB path: " + lora.GetDBPath() + 
							"\n\nFirst demo build\nAndrew Senko")
					.setCancelable(false)
					.setNegativeButton("Close", new DialogInterface.OnClickListener() {
						
						@Override
						public void onClick(DialogInterface dialog, int which) {
							dialog.cancel();							
						}
					});
			AlertDialog alert = builder.create();
			alert.show();
			break;
			
		default:
			break;
		}
		return super.onOptionsItemSelected(item);
	}
}
