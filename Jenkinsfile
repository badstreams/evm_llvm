pipeline {
  agent none
  stages {
    stage('linux') {
      steps {
        sh '''rm -rf evmbuild
'''
        sh '''cat /proc/cpuinfo
whoami'''
        sh 'mkdir -p evmbuild && cd evmbuild && sudo cmake $WORKSPACE -DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD=EVM && sudo make -j 8'
        sh 'rm -rf evmbuild'
      }
    }
  }
}