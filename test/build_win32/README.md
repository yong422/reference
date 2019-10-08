VC15 test for reference
=======================

# Descripton


# Warning

## 플랫폼 도구 집합 설정 주의

> VC15 (Visual studio 2017) 에서 gtest 프로젝트 사용을 위해서는 플랫폼 도구집합을 v141 (Visual Studio 2017) 로 해야 한다.   
> Windows 2003 이하의 windows os 에 프로그램 배포시 v141_xp (Visual Studio 2017 with Windows XP) 를 사용하여 빌드한다.   
> 테스트 프로젝트는 배포가 필요없으므로 v141 플랫폼 도구집합을 사용하도록 한다.   