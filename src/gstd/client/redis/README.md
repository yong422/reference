** v0.1.2

# Description

- c++ redis client
- 헤더, cpp 파일 각각 1개
- Depend on hiredis (https://github.com/redis/hiredis.git)


## API

### 접속
```c++
//            host, port, dbindex, timeout(ms), retries
redis::CClient* rcli = new redis::CClient("127.0.0.1", 6379, 0, 10000, 3);
rcli->SetDebug();   //! 디버깅 모드. 모든 성공, 실패 메시지 출력
rcli->SetAuth("testpasswd");   //
if(rcli->Connect()){
  // 연결성공
}else{
  // 연결실패
  fprintf(stdout, "%s\n", rcli->GetErrorMsg().c_str());
}

```

### Set
``` c++
if(!rcli->Set("key1", "string_val")){
  // 성공 
}else{
  // 실패
}
if(!rcli->Set("key2", (unsigned int)time(NULL))){
  // 성공
}else{
  // 실패
}
if(!rcli->Set("key3", (double)3.1456)){
  // 성공
}else{
  // 실패
}
   

```

## Debug

- leak check : valgrind --leak-check=full --show-reachable=yes ./redisclient
