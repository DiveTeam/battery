import { VOLTAGE2_CHANGE } from '../constants';

export function changeVoltage2(voltage) {
  return {
    type: VOLTAGE2_CHANGE,
    voltage2: voltage
  }
}
