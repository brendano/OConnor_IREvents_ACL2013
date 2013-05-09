import java.lang.reflect.Field;
import java.util.Properties;

import util.U;

public class UtilHere {
	public static void setField(Object obj, String fieldName, double value) throws SecurityException, NoSuchFieldException, IllegalArgumentException, IllegalAccessException {
		Field f = obj.getClass().getDeclaredField(fieldName);
		f.setDouble(obj, value);
	}
	public static void setField(Object obj, String fieldName, int value) throws SecurityException, NoSuchFieldException, IllegalArgumentException, IllegalAccessException {
		Field f = obj.getClass().getDeclaredField(fieldName);
		f.setInt(obj, value);
	}
	public static void setFieldDouble(Object obj, Properties props, String fieldName) {
		if (! props.containsKey(fieldName)) {
			return;
		}
		double x = Double.valueOf((String) props.getProperty(fieldName));
		try {
			setField(obj, fieldName, x);
		} catch (SecurityException e) {
			e.printStackTrace();
		} catch (IllegalArgumentException e) {
			e.printStackTrace();
		} catch (NoSuchFieldException e) {
			U.p(fieldName + " in properties but not class; skipping.");
		} catch (IllegalAccessException e) {
			e.printStackTrace();
		}
	}
	public static void setFieldInteger(Object obj, Properties props, String fieldName) {
		if (! props.containsKey(fieldName)) {
			return;
		}
		int x = Integer.valueOf((String) props.getProperty(fieldName));
		try {
			setField(obj, fieldName, x);
		} catch (SecurityException e) {
			e.printStackTrace();
		} catch (IllegalArgumentException e) {
			e.printStackTrace();
		} catch (NoSuchFieldException e) {
			U.p(fieldName + " in properties but not class; skipping.");
		} catch (IllegalAccessException e) {
			e.printStackTrace();
		}
	}

}
