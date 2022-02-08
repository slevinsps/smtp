import socket
import os
import glob
import shutil

CONNECT_ANS = b'220 Service ready\r\n'
# OK_ANS = b'250 OK\r\n'
OK_ANS = b'250 Requested mail action okay, completed\r\n'
DATA_ANS = b'354 Start mail input; end with <CRLF>.<CRLF>\r\n'
BAD_SEQ_COM_ANS = b'503 Bad sequence of commands\r\n'
CLOSING_CHANNEL_ANS = b'221 Service closing transmission channel\r\n'

def check_command(s, cmd, body, code):
    s.sendall(str.encode(f'{cmd}{body}\r\n'))
    data = s.recv(1024)
    if data != code:
        s.close()
        print(f'FAIL in {cmd}{body} recieve = |{data}|, expected = |{code}|')
        return -1
    return 0

def test_one_connect(path_to_data = None, num_rcpt = 1, num_mails = 1, sender = True, host = 'localhost', port = 1026):
    num_rcpt = max(num_rcpt, 1)
    send_data = b'hello world!'
    
    if path_to_data:
        with open(path_to_data, 'r') as content_file:
            send_data = str.encode(content_file.read())


    server_address = (host, port)             
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(server_address)
    
    data = s.recv(1024)
    if data != CONNECT_ANS:
        s.close()
        print(f'FAIL in connect recieve = |{data}|, expected = |{CONNECT_ANS}|')
        return -1
    
    if check_command(s, 'helo ', 'localhost', OK_ANS) < 0: return -1 
    for mail_ind in range(num_mails):
        if sender:
            if check_command(s, 'mail from: ', '<ivan@mail.ru>', OK_ANS) < 0: return -1 
        else:
            if check_command(s, 'mail from: ', '<>', OK_ANS) < 0: return -1 

        for i in range(num_rcpt):
            if check_command(s, 'rcpt to: ', f'<test{i}@mail.ru>', OK_ANS) < 0: return -1 
        if check_command(s, 'data', '', DATA_ANS) < 0: return -1 
        s.sendall(send_data)
        if check_command(s, '\r\n.', '', OK_ANS) < 0: return -1 
        
    if check_command(s, 'quit', '', CLOSING_CHANNEL_ANS) < 0: return -1 
    
    
    s.close()
    return 0

def test_on_bad_commands(host = 'localhost', port = 1026):
    send_data = b'hello world!'

    server_address = (host, port)             
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(server_address)
    
    data = s.recv(1024)
    if data != CONNECT_ANS:
        s.close()
        print(f'FAIL in connect recieve = |{data}|, expected = |{CONNECT_ANS}|')
        return -1
    
    if check_command(s, 'aaaaaaa', '', BAD_SEQ_COM_ANS) < 0: return -1 
    if check_command(s, 'helo ', 'localhost', OK_ANS) < 0: return -1 
    if check_command(s, 'mailllll from: ', '<ivan@mail.ru>', BAD_SEQ_COM_ANS) < 0: return -1 
    if check_command(s, 'mail from: ', '<ivan@mail.ru>', OK_ANS) < 0: return -1 
    if check_command(s, 'rcpt to  : ', '<test25@mail.ru>', BAD_SEQ_COM_ANS) < 0: return -1 
    if check_command(s, 'rcpt to: ', '<test25@mail.ru>', OK_ANS) < 0: return -1 
    if check_command(s, 'datadata', '', BAD_SEQ_COM_ANS) < 0: return -1 
    if check_command(s, 'data', '', DATA_ANS) < 0: return -1 
    s.sendall(send_data)
    if check_command(s, '\r\n.', '', OK_ANS) < 0: return -1 
    if check_command(s, '\r\n.', '', BAD_SEQ_COM_ANS) < 0: return -1 
    if check_command(s, 'quit', '', CLOSING_CHANNEL_ANS) < 0: return -1 
    
    s.close()
    return 0


def test_on_rset_commands(host = 'localhost', port = 1026):
    send_data = b'hello world!'

    server_address = (host, port)             
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(server_address)
    
    data = s.recv(1024)
    if data != CONNECT_ANS:
        s.close()
        print(f'FAIL in connect recieve = |{data}|, expected = |{CONNECT_ANS}|')
        return -1
    
    if check_command(s, 'helo ', 'localhost', OK_ANS) < 0: return -1  
    if check_command(s, 'mail from: ', '<ivan@mail.ru>', OK_ANS) < 0: return -1 
    if check_command(s, 'rset', '', OK_ANS) < 0: return -1 
    if check_command(s, 'mail from: ', '<ivan2@mail.ru>', OK_ANS) < 0: return -1 
    if check_command(s, 'rcpt to: ', '<test1@mail.ru>', OK_ANS) < 0: return -1 
    if check_command(s, 'rset', '', OK_ANS) < 0: return -1 
    if check_command(s, 'mail from: ', '<ivan3@mail.ru>', OK_ANS) < 0: return -1 
    if check_command(s, 'rcpt to: ', '<test2@mail.ru>', OK_ANS) < 0: return -1 
    if check_command(s, 'data', '', DATA_ANS) < 0: return -1 
    s.sendall(send_data)
    if check_command(s, '\r\n.', '', OK_ANS) < 0: return -1 
    
    if check_command(s, 'quit', '', CLOSING_CHANNEL_ANS) < 0: return -1 
    
    s.close()
    return 0


def test_on_quit_commands(host = 'localhost', port = 1026):
    send_data = b'hello world!'

    server_address = (host, port)             
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(server_address)
    data = s.recv(1024)
    if data != CONNECT_ANS:
        s.close()
        print(f'FAIL in connect recieve = |{data}|, expected = |{CONNECT_ANS}|')
        return -1
    if check_command(s, 'helo ', 'localhost', OK_ANS) < 0: return -1  
    if check_command(s, 'quit', '', CLOSING_CHANNEL_ANS) < 0: return -1 
    s.close()
    
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(server_address)
    data = s.recv(1024)
    if data != CONNECT_ANS:
        s.close()
        print(f'FAIL in connect recieve = |{data}|, expected = |{CONNECT_ANS}|')
        return -1
    if check_command(s, 'helo ', 'localhost', OK_ANS) < 0: return -1  
    if check_command(s, 'mail from: ', '<ivan@mail.ru>', OK_ANS) < 0: return -1 
    if check_command(s, 'quit', '', CLOSING_CHANNEL_ANS) < 0: return -1 
    s.close()
    
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(server_address)
    data = s.recv(1024)
    if data != CONNECT_ANS:
        s.close()
        print(f'FAIL in connect recieve = |{data}|, expected = |{CONNECT_ANS}|')
        return -1
    if check_command(s, 'helo ', 'localhost', OK_ANS) < 0: return -1  
    if check_command(s, 'mail from: ', '<ivan@mail.ru>', OK_ANS) < 0: return -1 
    if check_command(s, 'rcpt to: ', '<test1@mail.ru>', OK_ANS) < 0: return -1 
    if check_command(s, 'quit', '', CLOSING_CHANNEL_ANS) < 0: return -1 
    s.close()
    
    
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(server_address)
    data = s.recv(1024)
    if data != CONNECT_ANS:
        s.close()
        print(f'FAIL in connect recieve = |{data}|, expected = |{CONNECT_ANS}|')
        return -1
    if check_command(s, 'helo ', 'localhost', OK_ANS) < 0: return -1  
    if check_command(s, 'mail from: ', '<ivan@mail.ru>', OK_ANS) < 0: return -1 
    if check_command(s, 'rcpt to: ', '<test1@mail.ru>', OK_ANS) < 0: return -1 
    if check_command(s, 'data', '', DATA_ANS) < 0: return -1 
    s.sendall(send_data)
    if check_command(s, '\r\n.', '', OK_ANS) < 0: return -1
    if check_command(s, 'quit', '', CLOSING_CHANNEL_ANS) < 0: return -1 
    s.close()
    
    return 0

def tests():
    mail_path = './mail'
    if os.path.exists(mail_path):
        files = glob.glob(mail_path + '/*')
        for f in files:
            shutil.rmtree(f)
        
    # one rcpt small text
    res = test_one_connect(num_rcpt = 1)
    print('FAIL in test 1') if res < 0 else print('PASS test 1');
    # five rcpt
    res = test_one_connect(num_rcpt = 5)
    print('FAIL in test 2') if res < 0 else print('PASS test 2');
    # one rcpt bif text
    res = test_one_connect(num_rcpt = 1, path_to_data = './server/tests/big_text.txt')
    print('FAIL in test 3') if res < 0 else print('PASS test 3');
    # many mails from one client without quit
    res = test_one_connect(num_rcpt = 1, num_mails=20)
    print('FAIL in test 4') if res < 0 else print('PASS test 4');
    # bad_commands
    res = test_on_bad_commands()
    print('FAIL in test 5') if res < 0 else print('PASS test 5');
    # rset_test
    res = test_on_rset_commands()
    print('FAIL in test 6') if res < 0 else print('PASS test 6');
    # quit_test
    res = test_on_quit_commands()
    print('FAIL in test 7') if res < 0 else print('PASS test 7');
    # one rcpt small text no sender
    res = test_one_connect(num_rcpt = 1, sender = False)
    print('FAIL in test 8') if res < 0 else print('PASS test 8');
        
    
if __name__ == '__main__':
    tests()