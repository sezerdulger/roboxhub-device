#ifndef Updater_h
#define Updater_h
#include "Arduino.h"

class Updater
{
  public:
    Updater(String url);
    void beginUpdate();
  private:
    String _url;
};

#endif
