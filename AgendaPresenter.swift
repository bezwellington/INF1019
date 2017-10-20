//
//  AgendaPresenter.swift
//  Yoctor
//
//  Created by Lucas Ferraço on 12/05/17.
//  Copyright © 2017 YoctorTeam. All rights reserved.
//

import UIKit
import JTAppleCalendar

class AgendaPresenter: UIViewController, UITableViewDelegate, UITableViewDataSource {
	
	fileprivate var appointmentManager: AppointmentManager!
	private var personManager: PersonManager!
	fileprivate var selectedDayAppointments: [String : Appointment]!
	fileprivate var selectedDate: CustomDate!
	private var emptySolicitationLabel: UILabel!
	private var scheduleView: ScheduleAppointmentView!
	private var activityIndicatorView: UIView?
	
    @IBOutlet var whiteView: UIView!
    @IBOutlet var solicitationTable: UITableView!
    @IBOutlet weak var dayTable: UITableView!
	@IBOutlet weak var today: UILabel!
	@IBOutlet weak var calendarView: JTAppleCalendarView!
	@IBOutlet weak var monthYearLabel: UILabel!
	
	override func viewDidLoad() {
		super.viewDidLoad()
		
		appointmentManager = AppointmentManager.sharedInstance
		personManager = PersonManager.sharedInstance
		
		setupCalendarView()
        updateDayTable(forDate: CustomDate())

		dayTable.dataSource = self
		dayTable.delegate = self
		solicitationTable.dataSource = self
		solicitationTable.delegate = self
		
		emptySolicitationLabel = UILabel()
		view.addSubview(emptySolicitationLabel)
		
        whiteView.layer.shadowColor = UIColor.gray.cgColor
        whiteView.layer.shadowOpacity = 1
        whiteView.layer.shadowOffset = CGSize.zero
        whiteView.layer.shadowRadius = 1
        
        //Set the observe of all todayAppoitments
        appointmentManager.setAllObserversForTodayAppointments { (result) in
            if result == success {
                // aqui vc terá que chamar o activity 
                // depois terá que colocar a consulta que foi confirmada na fila de espera
            }
        }
	}
	
	fileprivate func updateDayTable(forDate date: CustomDate) {
		selectedDate = date

		selectedDayAppointments = [:]
		for (_, appointment) in appointmentManager.appointmentsOf(date: selectedDate) {
			selectedDayAppointments.updateValue(appointment, forKey: appointment.scheduleTime.timeDescription())
		}

//		calendarView.scrollToDate(date.nativeRepresentation())
		dayTable.reloadData()
	}
	
	//MARK: DayTable Delegate & DataSource
	func tableView(_ tableView: UITableView, viewForHeaderInSection section: Int) -> UIView? {
		if tableView == solicitationTable {
			let headerView = UIView()
			headerView.backgroundColor = UIColor(red: 242/255, green: 242/255, blue: 242/255, alpha: 1)
			headerView.frame = CGRect(x: 0, y: 0, width: tableView.bounds.size.width, height: 50)
			
			let label: UILabel = UILabel(frame: CGRect(x: 10, y: -2, width: tableView.bounds.size.width, height: 50))
			label.backgroundColor = UIColor.clear
			label.textColor = UIColor(red: 75/255, green: 74/255, blue: 74/255, alpha: 1)
			label.text = "Solicitações de Agendamento"
			
			headerView.addSubview(label)
			
			return headerView
		}
		else {
			return nil
		}
	}
	
	func tableView(_ tableView: UITableView, heightForHeaderInSection section: Int) -> CGFloat {
		if tableView == dayTable {
			return 0
		}
		else { // solicitationTable
			return 40
		}
	}
	
	func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
		if tableView == dayTable {
			// (Last schedule avaible - first schedule avaible)/(appointment duration) + 1(includes the first schedule hour)
			return Int((18.5 - 7)*2) + 1
		}
		else { // solicitationTable
			return 1
		}
	}
	
	func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
		var cell = UITableViewCell()
		
		if tableView == dayTable {
			let scheduledCell = tableView.dequeueReusableCell(withIdentifier: "ScheduledAppointmentCell", for: indexPath) as! ScheduledAppointmentCell
			let timeOfCell = CustomDate(fromDescription: "01-01-2017", "07-00").shifted(of: indexPath.row * 30).timeDescription()
			
			scheduledCell.time.text = timeOfCell.replacingOccurrences(of: "-", with: ":")
			scheduledCell.name.text = ""
			
			if let scheduledAppointment = selectedDayAppointments[timeOfCell] {
				scheduledCell.name.text = personManager.allPatients[scheduledAppointment.patientUID]?.name
				scheduledCell.time.font = UIFont.systemFont(ofSize: 17)
			}
			
			cell = scheduledCell
		}
		else { // solicitationTable
			cell.backgroundColor = .clear
			solicitationTable.isScrollEnabled = false
			
			emptySolicitationLabel.frame = solicitationTable.frame
			emptySolicitationLabel.textAlignment = .center
			emptySolicitationLabel.text = "Nenhuma solicitação de consulta no momento."
		}
		
		return cell
	}
    
    func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
		if tableView == dayTable {
			let timeOfCell = CustomDate(fromDescription: "01-01-2017", "07-00").shifted(of: indexPath.row * 30)
			
			if selectedDayAppointments[timeOfCell.timeDescription()] == nil { // Schedule new appointment
				scheduleView = ScheduleAppointmentView(frame: CGRect(x: view.frame.width/2 - 255,
				                                                     y: view.frame.height/2 - 117,
				                                                     width: 510, height: 234))
				
				let date = selectedDate
				date?.hour = timeOfCell.hour
				date?.minute = timeOfCell.minute
				
				var patientsNames = [String]()
				for (_, patient) in personManager.allPatients {
					patientsNames.append(patient.name)
				}
				
				scheduleView.button.addTarget(self, action: #selector(self.saveNewAppointment), for: .touchUpInside)
				scheduleView.showView(on: self, date: date!, data: patientsNames)
			}
		}
		else { // solicitationTable
			
		}
    }
	
	func saveNewAppointment(_ sender: UIButton) {
		let patientUID = personManager.allPatients.first { (_, patient) -> Bool in
			return patient.name == scheduleView.filteredData[sender.tag]
		}
		// FIX: get doctors dynamically
		let newAppointment = Appointment(patientUID: (patientUID?.key)!, doctorUID: "123456789", scheduleTime: scheduleView.selectedTime)
		
		activityIndicatorView = ActivityIndicator().startActivityIndicator(obj: self)
		appointmentManager.createNewAppointment(newAppointment: newAppointment) { (result) in
			if result == success {
				self.updateDayTable(forDate: CustomDate())
			}
			
			ActivityIndicator().stopActivityIndicator(obj: self, indicator: self.activityIndicatorView!)
		}
	}
	
}

//MARK:- JTAppleCalendar Extension
extension AgendaPresenter: JTAppleCalendarViewDelegate, JTAppleCalendarViewDataSource {
	func setupCalendarView() {
		//Setup calendar spacing
		calendarView.minimumLineSpacing = 0
		calendarView.minimumInteritemSpacing = 0
		
		//Setup labels
		calendarView.visibleDates { (visibleDates) in
			self.setupViewsOfCalendar(from: visibleDates)
		}
		
		var selectedDates = [Date]()
		selectedDates.append(Date())
		calendarView.selectDates(selectedDates)
        monthYearLabel.text = CustomDate().monthDescription()
	}
	
	func handleCellTextColor(view: JTAppleCell?, cellState: CellState) {
		guard let validCell = view as? MiniCalendarCell else { return }
		
		if cellState.dateBelongsTo == .thisMonth {
			validCell.dateLabel.textColor = monthColor
			
		} else {
			validCell.dateLabel.textColor = outsideMonthColor
		}
	}
	
	func handleCellSelected(view: JTAppleCell?, cellState: CellState) {
		guard let validCell = view as? MiniCalendarCell else { return }
        
		if cellState.isSelected {
			validCell.selectedView.isHidden = false
			validCell.dateLabel.textColor = monthColor
		} else {
			validCell.selectedView.isHidden = true
		}
	}
	
	func setupViewsOfCalendar(from visibleDates: DateSegmentInfo) {
		let date = visibleDates.monthDates.first!.date
		
		let formatter = DateFormatter()
		formatter.dateFormat = "MMMM yyyy"
        monthYearLabel.text = formatter.string(from: date).capitalized
	}
	
	func configureCalendar(_ calendar: JTAppleCalendarView) -> ConfigurationParameters {
		let formatter = DateFormatter()
		formatter.dateFormat = "yyyy MM dd"
		//formatter.timeZone = Calendar.current.timeZone
		//formatter.locale = Calendar.current.locale
		
		let startDate = Date()
		let endDate = Date().addingTimeInterval(5*12.0*30.0*24.0*60.0*60.0) //shows next 5 years
		
		let parameters = ConfigurationParameters(startDate: startDate,
		                                         endDate: endDate,
		                                         numberOfRows: 6,
		                                         calendar: Calendar.current,
		                                         generateInDates: .forAllMonths,
		                                         generateOutDates: .off,
		                                         firstDayOfWeek: .sunday)
		
		return parameters
	}
	
	func calendar(_ calendar: JTAppleCalendarView, cellForItemAt date: Date, cellState: CellState, indexPath: IndexPath) -> JTAppleCell {
		var cell = calendar.dequeueReusableJTAppleCell(withReuseIdentifier: "MiniCalendarCell", for: indexPath) as! MiniCalendarCell
		
		let today = Date()
		
		let dateFormatter = DateFormatter()
		dateFormatter.dateFormat = "yyyy MM dd" // or whatever format you want.
		
		let currentDateString = dateFormatter.string(from: today)
		let cellStateDateString = dateFormatter.string(from: cellState.date)
		
		//If date is less than or different from today, paint gray
		if(cellState.date < today && currentDateString !=  cellStateDateString ) {
			cell = calendar.dequeueReusableJTAppleCell(withReuseIdentifier: "MiniCalendarCell2", for: indexPath) as! MiniCalendarCell
			cell.backgroundColor = pastDayColor
		}
		
		cell.dateLabel.text = cellState.text
		
		handleCellSelected(view: cell, cellState: cellState)
		handleCellTextColor(view: cell, cellState: cellState)
		
		//Changing today's background color
		
		
		if  currentDateString ==  cellStateDateString && !cellState.isSelected {
			cell.dateLabel.textColor = todayBackgroundColor
		} else if cellState.dateBelongsTo == .thisMonth {
			cell.dateLabel.textColor = monthColor
		} else {
			cell.dateLabel.textColor = outsideMonthColor
		}
		//Day status
		
		//se dia está CHEIO...
		//        cell.statusView.isHidden = false
		//        cell.statusView.backgroundColor = fullStatusColor
		
		//se dia está MODERADO...
		//        cell.statusView.isHidden = false
		//        cell.statusView.backgroundColor = moderateStatusColor
		
        cell.frame.size = CGSize(width: 45, height: 45)
        cell.selectedView.frame.size = cell.frame.size
        cell.layer.cornerRadius = cell.frame.size.width/2
        cell.layer.borderWidth = 0.5
        cell.layer.borderColor = UIColor.clear.cgColor
        cell.layer.masksToBounds = true
        cell.selectedView.layer.cornerRadius = cell.frame.size.width/2
        cell.selectedView.layer.borderWidth = 0.5
        cell.selectedView.layer.borderColor = UIColor.clear.cgColor
        cell.selectedView.layer.masksToBounds = true
        
		return cell
	}
	
	func calendar(_ calendar: JTAppleCalendarView, didDeselectDate date: Date, cell: JTAppleCell?, cellState: CellState) {
		handleCellSelected(view: cell, cellState: cellState)
		handleCellTextColor(view: cell, cellState: cellState)
        calendarView.reloadData()
	}
	
	func calendar(_ calendar: JTAppleCalendarView, didSelectDate date: Date, cell: JTAppleCell?, cellState: CellState) {
		updateDayTable(forDate: CustomDate(fromNative: date))
		
		self.today.text = selectedDate.extendedDesc()
		
		handleCellSelected(view: cell, cellState: cellState)
		handleCellTextColor(view: cell, cellState: cellState)
        calendarView.reloadData()
	}
	
	func calendar(_ calendar: JTAppleCalendarView, didScrollToDateSegmentWith visibleDates: DateSegmentInfo) {
		setupViewsOfCalendar(from: visibleDates)
	}
}
