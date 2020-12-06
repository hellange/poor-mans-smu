#ifndef ANALOG_GAUGE_H
#define ANALOG_GAUGE_H


class AnalogGaugeClass {
  private:
    void showAnalogPin(int x, int y, int radius, int radiusStart, int degreeRelativeToTop, int needleColor, int lineWidth, bool needle);

  public:
    void renderAnalogGauge(int x, int y, int size, float degrees, float value, const char *title);
};

extern AnalogGaugeClass ANALOG_GAUGE;

#endif
