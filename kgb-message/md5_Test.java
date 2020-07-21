import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import javax.xml.bind.DatatypeConverter;

public class md5_Test{
    // method 1
     public static String calc_md5_1(String text) {
        String str = "";

        try {
            MessageDigest m = MessageDigest.getInstance("MD5");

            byte[] d = m.digest(text.getBytes());
            System.out.println(d.length);
            for (byte b : d) {
                str = str + String.format("%x", new Object[]{Byte.valueOf(b)});
            }

        } catch (NoSuchAlgorithmException x) {
            System.out.println("No md5");
        }

        return str;
     }

     // method 2
     public static String calc_md5_2(String text) {
        String str = "";

        try {
            MessageDigest m = MessageDigest.getInstance("MD5");

            // update() or digest() make no diff
            m.update(text.getBytes());
            byte[] d = m.digest();

            str = DatatypeConverter.printHexBinary(d);

        } catch (NoSuchAlgorithmException x) {
            System.out.println("No md5");
        }

        return str;
     }

     public static void main(String []args){
        
        System.out.println("Guest md5:");
        System.out.println(calc_md5_1("guest"));
        System.out.println(calc_md5_2("guest"));

        System.out.println("test md5:");
        System.out.println(calc_md5_1("test"));
        System.out.println(calc_md5_2("test"));

     }
}
