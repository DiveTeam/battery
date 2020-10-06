import { TEMPERATURE_CHANGE } from '../constants';

export function changeTemperature(temperature) {
  return {
    type: TEMPERATURE_CHANGE,
    temperature: temperature
  }
}
