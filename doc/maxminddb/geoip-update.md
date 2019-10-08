Geoip update 문서
=================

> 작성자 : 조용규<ykjo@gabia.com>   
> Maxmind 의 GeoIP2 Lite or GeoIP 를 업데이트 하기 위한 업데이트 툴.   
> repo https://github.com/maxmind/geoipupdate.git   

# 사용방법

* geoip/update 설치

  1. git 에서 최신 geoipupdate 를 복사한다.

    - update binary project

      > https://github.com/maxmind/geoipupdate.git   


  2. ./bootstrap;./configure={install dir};make;make install

      - prefix 기본값은 /usr/local 이며 하위에  {prefix}/bin/geoipupdate, {prefix}/etc/GeoIP.conf 파일이 생성된다.
      - database 저장폴더는 기본으로 {prefix}/share/GeoIP/ 가 설정되나 conf파일에서 변경이 가능하다.
      - etc/GeoIP.conf 경로는 컴파일시 고정되므로 prefix 설치시 주의.
      - geoipupdate 를 설치할 서버 /home/geoip 을 prefix 로 권장.

        > geoipupdate 실행파일이 GeoIP.conf 를 읽어오므로 설치 경로는 중요

  3. 구성은 다음과 같다. {prefix} 설치시

      - bin/geoipupdate   실행파일
      - etc/GeoIP.conf    설정파일
      - share/GeoIP/      database 파일이 저장될 directory

* 업데이트 설정

  * 상용 license를 구매한경우 GeoIP.conf 파일에 id와 key를 저장.
  * GeoIP.conf file - used by geoipupdate program to update databases
  *  from http://www.maxmind.com

    ```sh
    UserId YOUR_USER_ID_HERE   
    LicenseKey YOUR_LICENSE_KEY_HERE   
    ```

  1. 다운로드할 데이터베이스 파일을 다음과 같이 설정한다.

      ```bash
      # Include one or more of the following ProductIds:
      # * GeoLite2-City - GeoLite 2 City
      # * GeoLite2-Country - GeoLite2 Country
      # * GeoLite-Legacy-IPv6-City - GeoLite Legacy IPv6 City
      # * GeoLite-Legacy-IPv6-Country - GeoLite Legacy IPv6 Country
      # * 506 - GeoLite Legacy Country
      # * 517 - GeoLite Legacy ASN
      # * 533 - GeoLite Legacy City

      # 전체에 대한 예.
      # ProductIds GeoLite2-City GeoLite2-Country GeoLite-Legacy-IPv6-City GeoLite-Legacy-IPv6-Country 506 517 533

      ProductIds YOUR_PRODUCT_IDS_HERE # <-- 수정.
      ```

      - 예) 현재 사용하는 라이센스는 국가와 ISP 이므로

      ```bash
      ProductIds GeoLite2-Country # free
      ProductIds GeoIP2-Country   # license
      ProductIds GeoIP2-ISP       # license
      ```

  2. database 파일을 저장할 directory 설정.

      - GeoIP.conf file 의 다음값을 수정 (기본 주석처리 되어있으며 {prefix}/share/GeoIP/ 폴더에 저장됨)

      ```bash
      # DatabaseDirectory /usr/local/share/GeoIP
      ```

  3. crond 등록

      - 주기적인 업데이트를 위해 crond 에 등록한다.
      - MaxMind 에서는 상용은 1주일, GeoLite(free)는 1달을 주기로 업데이트한다.

      ```bash
      # top of crontab
      MAILTO=your@email.com
      4 9 * * 4 /usr/local/bin/geoipupdate
      # end of crontab
      ```
      - 5번째 필드가 요일값이며 0,7(일), 1(월) .. 순이다.