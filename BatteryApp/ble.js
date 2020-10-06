import React from 'react';
import { Buffer } from 'buffer';
import { BleManager } from 'react-native-ble-plx';
import {
  Platform,
  PermissionsAndroid
} from 'react-native';
import * as Constants from './constants'
import { changeVoltage1 } from './actions/voltage1Change';
import { changeVoltage2 } from './actions/voltage2Change';
import { changeVoltage3 } from './actions/voltage3Change';
import { changeSOC } from './actions/socChange';
import { changeTemperature } from './actions/temperatureChange';
import { changeCurrent } from './actions/currentChange';
import { connect } from "react-redux";
import { bindActionCreators } from 'redux';


function getFloatFromValue(readValueInBase64){
  const bytes = Buffer.from(readValueInBase64, 'base64');
  const buffer = new ArrayBuffer(4);
  let dview = new DataView(buffer);
  bytes.forEach(function (b, i) {
    dview.setUint8(i, b);
  });
  return dview.getFloat32(0, true)
}


function getByteFromValue(readValueInBase64){
  const bytes = Buffer.from(readValueInBase64, 'base64');
  return bytes[0]
}


class BleComponent extends React.PureComponent {
  constructor(props) {
    super(props);
    // this.state = { voltage1: 0 };
    if (Platform.OS === 'android') {
      PermissionsAndroid.request(PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION)
        .then(granted => {
          if (granted !== PermissionsAndroid.RESULTS.GRANTED) {
            console.log('BLE access denied');
            //TODO: throw some event to let the user know he had fucked up
          }
          else {
            this.initializeManager();
          }
        })
    } else {
      this.initializeManager();
    }
  }

  initializeManager() {
    this.manager = new BleManager({});
    const subscription = this.manager.onStateChange((state) => {
      if (state === 'PoweredOn') {
        this.scanAndConnect();
        subscription.remove();
      }
    }, true);
  }

  scanAndConnect() {
    this.manager.startDeviceScan([Constants.CANISTER_UUID], null, (error, device) => {
      console.log("Found: " + device.name);
      if (error) {
        console.log("BLE error: " + error);
        return
      }
      this.manager.stopDeviceScan();

      device.connect()
        .then((device) => {
          console.log("Connected to device: " + device.name);
          return device.discoverAllServicesAndCharacteristics();
        })
        .then((device) => {
          this.registerServices(device);
        })
        .catch((error) => {
           console.log("BLE connection error: " + error);
        });
    });
  }

  registerServices(device) {
    device.readCharacteristicForService(
      Constants.VOLTAGES_SERVICE_UUID,
      Constants.VOLTAGE1_CHARACTERISTIC_UUID
    ).then( (characteristic) => {

      characteristic.monitor((error, c) => {
        this.v1Listener(c);
      })
    });

    device.readCharacteristicForService(
      Constants.VOLTAGES_SERVICE_UUID,
      Constants.VOLTAGE2_CHARACTERISTIC_UUID
    ).then( (characteristic) => {

      characteristic.monitor((error, c) => {
        this.v2Listener(c);
      })
    });

    device.readCharacteristicForService(
      Constants.VOLTAGES_SERVICE_UUID,
      Constants.VOLTAGE3_CHARACTERISTIC_UUID
    ).then( (characteristic) => {

      characteristic.monitor((error, c) => {
        this.v3Listener(c);
      })
    });

    device.readCharacteristicForService(
      Constants.BATTERY_SERVICE_UUID,
      Constants.CURRENT_CHARACTERISTIC_UUID
    ).then( (characteristic) => {

      characteristic.monitor((error, c) => {
        this.currentListener(c);
      })
    });

    device.readCharacteristicForService(
      Constants.BATTERY_SERVICE_UUID,
      Constants.TEMPERATURE_CHARACTERISTIC_UUID
    ).then( (characteristic) => {

      characteristic.monitor((error, c) => {
        this.temperatureListener(c);
      })
    });

    device.readCharacteristicForService(
      Constants.BATTERY_SERVICE_UUID,
      Constants.BATTERY_LEVEL_UUID
    ).then( (characteristic) => {

      characteristic.monitor((error, c) => {
        this.SOCListener(c);
      })
    });
  }

  v1Listener(characteristic) {
    let v1 = getFloatFromValue(characteristic.value);
    this.props.changeVoltage1(v1);
    console.log("Voltage 1: " + v1.toFixed(3));
  }

  v2Listener(characteristic) {
    let v2 = getFloatFromValue(characteristic.value);
    this.props.changeVoltage2(v2);
    console.log("Voltage 2: " + v2.toFixed(3));
  }

  v3Listener(characteristic) {
    let v3 = getFloatFromValue(characteristic.value);
    this.props.changeVoltage3(v3);
    console.log("Voltage 3: " + v3.toFixed(3));
  }

  currentListener(characteristic) {
    let current = getFloatFromValue(characteristic.value);
    this.props.changeCurrent(current);
    console.log("Current: " + current.toFixed(3));
  }

  temperatureListener(characteristic) {
    let t = getFloatFromValue(characteristic.value);
    this.props.changeTemperature(t);
    console.log("Temperature: " + t.toFixed(3));
  }

  SOCListener(characteristic) {
    let soc = getByteFromValue(characteristic.value);
    this.props.changeSOC(soc);
    console.log("SOC: " + soc);
  }

  render() {
    return null;
  }
}

// const mapStateToProps = state => ({
//   voltage1: state.voltage1,
// });

// const ActionCreators = Object.assign(
//   {},
//   changeVoltage1,
// );

// const mapDispatchToProps = dispatch => ({
//   actions: bindActionCreators(ActionCreators, dispatch)
// });

// export default connect(mapStateToProps, mapDispatchToProps)(BleComponent)
export default connect(null, {
  changeVoltage1,
  changeVoltage2,
  changeVoltage3,
  changeSOC,
  changeTemperature,
  changeCurrent
})(BleComponent);
