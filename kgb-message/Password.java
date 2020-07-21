import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import javax.xml.bind.DatatypeConverter;
import java.io.PrintWriter;

public class Password{
    public static String r_b(String str) {
        char[] charArray = str.toCharArray();
        System.out.println(charArray.length);
        String flag = "";

        for (int i2 = 0; i2 < charArray.length / 2; i2++) {
            char c = charArray[i2];
            charArray[i2] = charArray[(charArray.length - i2) - 1];
            charArray[(charArray.length - i2) - 1] = c;
        }
        
        for (int i = 0; i < charArray.length; i++) {
            for (int j = 0; j < 256; j++) {
                char c = (char) j;
                if (((c >> (i % 8)) ^ c) == charArray[i]) {
                    flag += String.valueOf(c);   
                    break;
                }
            }
        }

        return flag; 
    }
    
    public static String r_a(String str) {
        char[] charArray = str.toCharArray();
        for (int i = 0; i < charArray.length / 2; i++) {
            char c = charArray[i];
            charArray[i] = (char) (charArray[(charArray.length - i) - 1] ^ 'A');
            charArray[(charArray.length - i) - 1] = (char) (c ^ '2'); 
        }
        return new String(charArray);
    }

    public static String a(String str) {
        char[] charArray = str.toCharArray();
        for (int i = 0; i < charArray.length / 2; i++) {
            char c = charArray[i];
            charArray[i] = (char) (charArray[(charArray.length - i) - 1] ^ '2');
            charArray[(charArray.length - i) - 1] = (char) (c ^ 'A');
        }
        return new String(charArray);
    }

    public static String b(String str) {
        char[] charArray = str.toCharArray();
        for (int i = 0; i < charArray.length; i++) {
            charArray[i] = (char) ((charArray[i] >> (i % 8)) ^ charArray[i]);
        }
        for (int i2 = 0; i2 < charArray.length / 2; i2++) {
            char c = charArray[i2];
            charArray[i2] = charArray[(charArray.length - i2) - 1];
            charArray[(charArray.length - i2) - 1] = c;
        }
        return new String(charArray);
    }

    public static String i(String q, String s) {
        char[] charArray = q.substring(19).toCharArray();
        charArray[1] = (char) (charArray[1] ^ 'U');
        charArray[2] = (char) (charArray[2] ^ 'F');
        charArray[3] = (char) (charArray[3] ^ 'F');
        charArray[5] = (char) (charArray[5] ^ '_');
        char[] charArray2 = s.substring(7, 13).toCharArray();
        System.out.println(charArray2[1]);
        System.out.println(charArray2[2]);
        System.out.println(charArray2[3]);
        charArray2[1] = (char) (charArray2[1] ^ '}');
        charArray2[2] = (char) (charArray2[2] ^ 'v');
        charArray2[3] = (char) (charArray2[3] ^ 'u');
        return new String(charArray) + "_" + new String(charArray2);
    }

    public static void main(String []args) {
        String p = "V@]EAASB\u0012WZF\u0012e,a$7(&am2(3.\u0003";
        String plt_p = r_a(p);
        
        System.out.println(plt_p);
        System.out.println(plt_p.length());

        String r = "\u0000dslp}oQ\u0000 dks$|M\u0000h +AYQg\u0000P*!M$gQ\u0000";  
        System.out.println(r);
        System.out.println(r.length());

        String f = r_b(r);
        System.out.println(f);
        System.out.println(f.length());
        
        String q = "Boris, give me the password";
        //String s = "Xay I *PXEASE* hXve the XasswordX";
        String s = "Hay I *PLEASE* have the passwordX";
        String flag = i(q, s);
        System.out.println(flag);
    }
}
