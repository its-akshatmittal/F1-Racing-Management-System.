import smtplib
import sys
import os
from email.mime.text import MIMEText
from dotenv import load_dotenv

load_dotenv()

def send_otp_email(receiver_email, otp):
    sender_email = os.getenv("LIBRARY_EMAIL")
    password = os.getenv("LIBRARY_EMAIL_PASSWORD")
    smtp_server = "smtp.gmail.com"
    port = 465  # For SSL

    message = MIMEText(f"Your OTP is: {otp}\nThis OTP is valid for 5 minutes.")
    message['Subject'] = "Password Reset OTP"
    message['From'] = sender_email
    message['To'] = receiver_email

    try:
        with smtplib.SMTP_SSL(smtp_server, port) as server:
            server.login(sender_email, password)
            server.sendmail(sender_email, receiver_email, message.as_string())
        return True
    except Exception as e:
        print(f"Error: {str(e)}", file=sys.stderr)
        return False

if __name__ == "__main__":
    if len(sys.argv) == 3:
        send_otp_email(sys.argv[1], sys.argv[2])