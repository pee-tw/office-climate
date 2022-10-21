import { useStore } from "@nanostores/react";
import axios from "axios";
import dayjs from "dayjs";
import localizedFormat from "dayjs/plugin/localizedFormat";
import { useEffect, useState } from "react";
import { SensorTemp } from "../store/sensor-temp";

dayjs.extend(localizedFormat);


interface WeatherResponse {
  Temperature: Temperature;
  Link: string;
  PrecipitationType?: any;
  EpochTime: number;
  IsDayTime: boolean;
  WeatherIcon: number;
  WeatherText: string;
  HasPrecipitation: boolean;
  retrieved: Retrieved;
  LocalObservationDateTime: string;
  MobileLink: string;
}

interface Retrieved {
  _seconds: number;
  _nanoseconds: number;
}

interface Temperature {
  Imperial: Imperial;
  Metric: Imperial;
}

interface Imperial {
  Unit: string;
  UnitType: number;
  Value: number;
}

const Outside = () => {
  const [weatherData, setWeatherData] = useState<WeatherResponse>(null);

  const sensorTemp = useStore(SensorTemp);

  useEffect(() => {
    axios
      .get(
        `https://asia-southeast1-office-sensors-27e21.cloudfunctions.net/weather`
      )
      .then(({ data }) => {
        const currentConditions = data as WeatherResponse;
        setWeatherData(currentConditions);
      });
  }, []);

  if (!weatherData) return <></>;

  const weatherIcon = () => {
    const { WeatherIcon } = weatherData;
    const iconNumberString =
      WeatherIcon.toString().length === 1
        ? `0${WeatherIcon}`
        : WeatherIcon.toString();

    return `https://developer.accuweather.com/sites/default/files/${iconNumberString}-s.png`;
  };

  const localTime = dayjs(weatherData.LocalObservationDateTime).format("LTS");

  const isFresh = dayjs(weatherData.LocalObservationDateTime).isSame(
    dayjs(),
    "hour"
  );

  const outsideTemperature = weatherData.Temperature.Metric.Value;

  const dayTimeString = weatherData.IsDayTime
    ? "The sun is still shining"
    : "The sun has set";

  const tempDelta = Math.abs(sensorTemp - outsideTemperature);
  const deltaStyle =
    tempDelta > 10
      ? {
          color: "red",
        }
      : {};

  return (
    <div className="flex flex-col items-center">
      <h2 className="mt-3">The Weather Outside</h2>
      <img src={weatherIcon()} alt="" className="src" />
      <p>It is: {weatherData.WeatherText}</p>
      {weatherData.HasPrecipitation && <p>It is currently raining</p>}
      <p>{dayTimeString}</p>
      <p>Temp: {outsideTemperature}</p>
      {isFresh && (
        <p style={deltaStyle}>
          The temperature difference is: {tempDelta.toFixed(2)}
        </p>
      )}
      <p>As of: {localTime}</p>
      <a className="text-blue-500" href={weatherData.MobileLink}>
        Powered by AccuWeather
      </a>
    </div>
  );
};

export default Outside;
