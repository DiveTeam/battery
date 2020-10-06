import { VOLTAGE3_CHANGE } from '../constants';

export function changeVoltage3(voltage) {
  return {
    type: VOLTAGE3_CHANGE,
    voltage3: voltage
  }
}
