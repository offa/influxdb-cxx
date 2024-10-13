// Sample project to check if the deployment process works
#include <InfluxDB/InfluxDBFactory.h>

int main()
{
    auto influxdb = influxdb::InfluxDBFactory::Get("http://localhost:8086?db=test");
    influxdb->write(influxdb::Point{"test"}.addField("value", 10));
}
