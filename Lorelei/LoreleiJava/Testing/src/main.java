/**
 * Created by andrey on 13.08.14.
 */
public class main {
    public static void main(String args[]){
        System.out.println(If(true));
        System.out.println(If(false));

    }


    public static String If(Boolean inf) {
            if(inf)
                return "In if";
           return "Not in if";
    }
}
