//
//  AppointmentManager.swift
//  Yoctor_Patient
//
//  Created by Wellington Bezerra on 06/09/17.
//  Copyright © 2017 Wellington Bezerra. All rights reserved.
//

import UIKit

class AppointmentManager {
	
	private let appointmentsDAO: DAO_Appointment!
    private let loginManager: LoginManager!
    public var allAppointments: [String : Appointment]!
	public var currentAppointment: Appointment!
	
	//MARK: Singleton Definition
	private static var theOnlyInstance: AppointmentManager?
	static var sharedInstance: AppointmentManager? {
		get {
			if theOnlyInstance == nil {
				theOnlyInstance = AppointmentManager()
			}
			return theOnlyInstance!
		}
	}
    
    private init() {
        appointmentsDAO = DAO_Appointment.sharedInstance
        loginManager = LoginManager.sharedInstance
        
        allAppointments = [String : Appointment]()
		currentAppointment = nil
    }
	
	//MARK:- Fetch Functions
    public func getAppointments( _ completion: @escaping ((_ result: String) -> Void)) {
		currentAppointment = nil
		
        self.appointmentsDAO.fetchAppointments(ofClinic: (loginManager.user?.appointmentsTable)!) { (appointments,result) in
            if result == success {
                for oneAppointment in appointments {
                    let appoint = Appointment(json: oneAppointment.value as! [String : Any])!
                    self.allAppointments.updateValue(appoint, forKey: oneAppointment.key)
					
					if appoint.status == .inAppointment {
						self.currentAppointment = appoint
					}
				}
				completion(success)
			}
			else {
				completion(result)
			}
		}
    }
    
    //MARK:- Organize Appointments
    

    /// - Parameters:
    ///     - doctor: uid of doctor
    ///
    /// - Returns: 
    ///     - dictionary of appointments of a doctor
    func appointmentsOf(doctor: String) -> [String : Appointment] {
        var appointmentsOfDoctor = [String : Appointment]()
        
        for appointment in allAppointments {
            if (appointment.value.doctorUID == doctor){
                appointmentsOfDoctor.updateValue(appointment.value, forKey: appointment.key)
            }
        }
        return appointmentsOfDoctor
    }
    
    /// - Parameters:
    ///     - date: date with day, month and year
    ///
    /// - Returns:
    ///     - dictionary of appointments of a date
    func appointmentsOf(date: CustomDate) -> [String : Appointment] {
        var appointmentsOfOneDate = [String : Appointment]()
        
        for (key, appointment) in allAppointments {
            if appointment.scheduleTime.compareDate(with: date) == .equal {
                appointmentsOfOneDate.updateValue(appointment, forKey: key)
            }
        }
        
        return appointmentsOfOneDate
    }
    
    /// - Parameters:
    ///     - array: array of appointments
    ///
    /// - Returns:
    ///     - array of sorted appointments
    public func sortedByHour(_ array: [Appointment]) -> [Appointment] {
        var appointments = [Appointment] ()
		appointments = array.sorted(by: { (first, second) -> Bool in
			return first.scheduleTime.compareTime(with: second.scheduleTime) == .bigger
		})
		
        return appointments
    }
	
	//MARK:- Update Functions
	public func saveDuration(of appointment: Appointment, with duration: Int, _ completion: @escaping ((_ result: String) -> Void)) {
		let appointmentUID = allAppointments.flatMap { (key: String, val: Appointment) -> String? in
			appointment == val ? key : nil
		}
		let hours = duration / 3600
		let hoursString = String(format: "%02d", hours)
		let minutes = duration/60 - hours*60
		let minutesString = String(format: "%02d", minutes)
		let seconds = duration - minutes*60
		let secondsString = String(format: "%02d", seconds)
		let durationString = hoursString + "-" + minutesString + "-" + secondsString
		
		appointmentsDAO.saveDuration(of: appointmentUID[0], on: "appointment_prevmama", with: durationString, completion)
	}
	
	public func changeStatus(of appointment: Appointment, to status: Status, _ completion: @escaping ((_ result: String) -> Void)) {
		let appointmentUID = allAppointments.flatMap { (key: String, val: Appointment) -> String? in
			appointment == val ? key : nil
		}
		
		appointmentsDAO.updateStatus(of: appointmentUID[0], on: "appointment_prevmama", to: status.rawValue) { (result) in
			if result == success {
				self.allAppointments[appointmentUID[0]]?.status = status
			}
			
			completion(result)
		}
	}
    
    //MARK:- Create Functions
    public func createNewAppointment(newAppointment: Appointment, _ completion: @escaping ((_ result: String) -> Void)) {
        self.appointmentsDAO.saveNewAppointment(newAppointment.makeJSON(), { (result, appointmentUID) in
			if result == success {
				self.allAppointments.updateValue(newAppointment, forKey: appointmentUID)
			}
			
			completion(result)
		})
    }
    
    //MARK:- Observe
    public func setAllObserversForTodayAppointments(_ completion: @escaping ((_ result: String) -> Void)) {
        for keyAndAppointment in self.todayAppointmentsDic(){
            self.setObserverTodayAppointment(tableNameOfClinic: "appointment_prevmama", keyOfAppointment: keyAndAppointment.key)  { (result) in
                if result == success && keyAndAppointment.key == self.allAppointments.reversed().first?.key {
                    completion(success)
                }
                else {
                    completion(result)
                }
            }
        }
    }
    
    public func setObserverTodayAppointment(tableNameOfClinic: String,  keyOfAppointment: String, _ completion: @escaping ((_ result: String) -> Void)) {
        
        self.appointmentsDAO.observerAppointment(withKey: keyOfAppointment, onClinic: tableNameOfClinic) { (infoChangedOfAppointment,result) in
            // prediction time updated
            if result == success {
                //infoChangedOfAppointment = [ key of firebase : value of key ]
                //exemple: [schedule_date : 12-02-2017 ] or [schedule_time : 01-22 ]
                for oneInfo in infoChangedOfAppointment {
                    if oneInfo.key == "status"{
                        print("status atual da consulta do dia", oneInfo)
                        //oneInfo.value: ex: confirmed, notConfirmed, ...
                    }
                }
                completion(success)
            }
        }
    }
    
    
    //MARK:- Separate Appointments Functions
    
    private func todayAppointmentsDic() -> [String : Appointment] {
        var todayAppointments = [String : Appointment]()
        
        for (key, appointment) in allAppointments {
            if appointment.scheduleTime.compareDate(with: CustomDate()) == .equal {
                todayAppointments.updateValue(appointment, forKey: key)
            }
        }
        
        return todayAppointments
    }
	
}
