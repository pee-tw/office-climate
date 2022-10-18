import axios from "axios";
import dayjs from "dayjs";
import localizedFormat from "dayjs/plugin/localizedFormat";
import { useEffect, useState } from "react";

dayjs.extend(localizedFormat);

interface WeatherResponse {
  LocalObservationDateTime: string;
  EpochTime: number;
  WeatherText: string;
  WeatherIcon: number;
  HasPrecipitation: boolean;
  PrecipitationType?: any;
  IsDayTime: boolean;
  Temperature: Temperature;
  MobileLink: string;
  Link: string;
}

interface Temperature {
  Metric: Metric;
  Imperial: Metric;
}

interface Metric {
  Value: number;
  Unit: string;
  UnitType: number;
}

const Outside = () => {
  const locationKey = "3558711";
  const apiKey = "q3yTIkBnN5qNfbzprZq2Ce4jzUH40GA4";
  const [temp, setTemp] = useState(0);
  const [asOf, setAsof] = useState("");
  const [weather, setWeather] = useState("");
  const [weatherSrc, setWeatherSrc] = useState("");

  useEffect(() => {
    axios
      .get(
        `https://dataservice.accuweather.com/currentconditions/v1/${locationKey}?apikey=${apiKey}`
      )
      .then(({ data }) => {
        const currentConditions = data[0] as WeatherResponse;
        setTemp(currentConditions.Temperature.Metric.Value);
        setAsof(
          dayjs(currentConditions.LocalObservationDateTime).format("LTS")
        );
        setWeather(currentConditions.WeatherText);
        setWeatherSrc(currentConditions.MobileLink)
      });
  }, []);

  return (
    <>
      <h2 className="mt-3">The Weather Outside</h2>
      <p>{weather}</p>
      <p>Temp: {temp}</p>
      <p>As of: {asOf}</p>
      <a className="text-blue-500" href={weatherSrc}>Powered by</a>
    </>
  );
};

export default Outside;
