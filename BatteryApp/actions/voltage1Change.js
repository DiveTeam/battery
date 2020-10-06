import { VOLTAGE1_CHANGE } from '../constants';

export function changeVoltage1(voltage) {
  return {
    type: VOLTAGE1_CHANGE,
    voltage1: voltage
  }
}
