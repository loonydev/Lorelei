package ua.senko.lorelei;

import java.util.ArrayList;

import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;

enum  TokenType
{
	Word, Punct, None
} 

class Connection
{
	public String keyword, handler;
	public String obj_refs_str;
}

class Token
{
    public String term;
    public TokenType type;
}

class Command 
{
	public boolean is_object = false;
	public Connection base;

	public ArrayList<Command> args;
}

public class Lorelei
{
	private SQLiteDatabase _lorelei_db;

	private final String _name_short = "лора";
    private final String _name_long = "лорелей";

    private String _db_path;

    private final String _version = "indev-2.1j";

    private String _request;

    private ArrayList<Command> _current_commands = new ArrayList<Command>();

    private Context _context;
    
    private DataBaseHelper _dbh;
    
    private IPrinter _printer;

    public Lorelei(Context context)
    {
    	_context = context;
    	_db_path = "lorelei.db";
    }

    public void Init()
    {
    	_dbh = new DataBaseHelper(_context, _db_path);
    	_lorelei_db = _dbh.openDataBase();
    }

    public void ClearCommands()
    {
    	_current_commands.clear();
    }

    public String GetShortName()
    {
    	return _name_short;
    }

    public String GetLongName()
    {
    	return _name_long;
    }

    public String GetDBPath()
    {
    	return _dbh.DB_PATH + _db_path;
    }

    public String GetVersion()
    {
    	return _version;
    }

    public void SetDBPath(String db_path)
    {
    	_db_path = db_path;
    }

    public void Parse(String request)
    {
        _request = request;

        ArrayList<Token> request_vect = tokenize(); 
        analyze(request_vect);
    }

    public void SetPrinter(IPrinter printer)
    {
    	_printer = printer;
    }

    public void PrintResults()
    {
        for(Command com : _current_commands)
            printCommand(com);
        _printer.PrintDebugLog("\n");
    }
    
    public void Close()
    {
    	_lorelei_db.close();
    	_dbh.close();
    }

    private ArrayList<Token> tokenize()
    {
        ArrayList<Token> result = new ArrayList<Token>();
        Token current_token = new Token();
        current_token.term = "";

        for(char ch : _request.toCharArray())
        {
        	if(Character.isLetter(ch) || (""+ch).matches("[а-яА-Я]"))
        	{
                if(current_token.type == TokenType.Punct)
                    current_token.term = "";

                current_token.type = TokenType.Word;
                if((""+ch).matches("[А-Я]"))
                {
                	String capitals = "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ";
                	String lowers = "абвгдеёжзийклмнопрстуфхцчшщъыьэюя";
                	current_token.term += lowers.charAt(capitals.indexOf(ch));
                }
                else
                	current_token.term += Character.toLowerCase(ch);
        	}
            else if(".,:;?!".contains(""+ch))
            {
                if(current_token.type == TokenType.Word)
                {
                    result.add(current_token);
                    current_token = new Token();
                }

                current_token.type = TokenType.Punct;
                current_token.term += ch;
            }
            else 
            {
                if(current_token.type == TokenType.Word) 
                {
                    result.add(current_token);
                    current_token = new Token();
                }

                current_token.term = ""; 
                current_token.type = TokenType.None; 
            }
        }
       
        if(current_token.type == TokenType.Word) 
        {
            result.add(current_token);
            current_token = new Token();
        }
        
        return result;
    }

    private void analyze(ArrayList<Token> sentence)
    {
        if(sentence.size() == 0) 
            return;

        if(isName(sentence.get(0)) || isWeakword(sentence.get(0)))
            sentence.remove(0);

        eval(sentence, 0);
    }

    private void eval(ArrayList<Token> sentence, int current_id)
    {
    	eval(sentence, current_id, false, null);
    }

    private void eval(ArrayList<Token> sentence, int current_id, boolean arg_search, Command last_com)
    {
	    if(current_id == sentence.size())
	        return;
	
	    Token current = sentence.get(current_id);

    	//_printer.PrintDebugLog("Current term: " + current.term + "\n");
	
	    if(isName(current) || isWeakword(current))
	    {
	        sentence.remove(current_id);
	        eval(sentence, current_id, arg_search, last_com);
	        if(sentence.size() == 0)
	            return;
	    }
	
	    if(isKeyword(current))
	    {
	        if(arg_search)
	        {
	            last_com.args.add(getCommand(current));
	            last_com = last_com.args.get(last_com.args.size() - 1); 
	        }
	        else
	        {
	            _current_commands.add(getCommand(current));
	            last_com = _current_commands.get(_current_commands.size() - 1);
	        }
	        sentence.remove(current_id);
	
	        eval(sentence, current_id, true, last_com);
	    }
	
	    if(arg_search)
	    {
	    	String sql_request = "SELECT * FROM objects WHERE id IN (" + last_com.base.obj_refs_str
                    +") AND name = '" + current.term + "'";
	    	Cursor c = _lorelei_db.rawQuery(sql_request, null);
	    	if (c.moveToFirst()) 
	    	{
	            Command arg = new Command();
	            arg.base = new Connection();
	            arg.is_object = true;
	            arg.base.keyword = current.term;
	            last_com.args.add(arg); 
	            sentence.remove(current_id); 
	            eval(sentence, current_id);
	    	}
	    	c.close();
	    }
    }

    private boolean isName(Token current)
    {
        if(current.term.equals(_name_long) || current.term.equals(_name_short))
            return true;
        return false;
    }
    
    private boolean isWeakword(Token current)
    {
    	String sql_request = "SELECT word FROM weakwords WHERE word = '" + current.term + "'";
    	Cursor cursor = _lorelei_db.rawQuery(sql_request, null);
    	boolean success = cursor.moveToFirst(); 
    	cursor.close();
    	return success;
    }
    
    private boolean isKeyword(Token current)
    {
    	String sql_request = "SELECT keyword FROM commands WHERE keyword = '" + current.term + "'";
    	Cursor cursor = _lorelei_db.rawQuery(sql_request, null);
    	boolean success = cursor.moveToFirst(); 
    	cursor.close();
    	return success;
    }

    private Command getCommand(Token current)
    {
        Command command = null;

        String sql_request = "SELECT * FROM commands WHERE keyword = '" + current.term + "'";
    	Cursor cursor = _lorelei_db.rawQuery(sql_request, null);
    	if (cursor.moveToFirst()) 
    	{
            command = new Command();
            command.base = new Connection();
            command.base.keyword = cursor.getString(cursor.getColumnIndex("keyword"));
            command.base.handler = cursor.getString(cursor.getColumnIndex("handler"));
            command.base.obj_refs_str = cursor.getString(cursor.getColumnIndex("refs"));
            command.args = new ArrayList<Command>();
    	}
    	cursor.close();

        if(command != null)
            return command;
        _printer.PrintDebugLog("Can't find connection " + current.term + "\n");
        return null;
    }

    private void printCommand(Command com)
    {
        _printer.PrintDebugLog(com.base.handler + " ( ");
        for(Command arg : com.args)
        {
            if(!arg.is_object)
                printCommand(arg);
            else
            {
            	_printer.PrintDebugLog(arg.base.keyword + " ");
            }
        }
        _printer.PrintDebugLog(")");
    }
}

